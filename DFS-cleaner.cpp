/*This is a DFS-cleaner simulation program,
which simulates how a cleaner can traverses an enclosed room while it does not know the map of the room at the beginning.
You can rewrite the set_environment() and go_and_clean::print_obstacle_route_and_car() to remove the API provided by opencv library.
Instead of presenting the DFS-cleaning process by video, you can cout the obstacle, cleaning status, car postion e.t.c..*/

#include <windows.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <string>
#include "stack.h"
#include <cmath>

#define PI 3.1415926
using namespace cv;
using namespace std;

Mat show_image;
Mat global_animation_image;
Mat local_animation_image;
VideoWriter global_output;
VideoWriter local_output;
int scale;

int nr, nc;
int** environment;
int at_environment_r;
int at_environment_c;

class node
{
//"node" represents each unit area in the real world,
//which can be easily classified by 2 member variables, 
//cleaned_or_not and obstacle_or_not, to detemined whether to clean that area or not
public:
	int cleaned_or_not = 0;
	int obstacle_or_not = 0;
};
node** surrounding;
void instruction()
{
	cout << endl;
	cout << "****************************************************************************************************" << endl;
	cout << "----------------------------------------------------------------------------------------------------" << endl;
	cout << "# INSTRUCTION" << endl << endl;
	cout << "	This is a DFS-cleaner simulation program," << endl;
	cout << "	which simulates how a cleaner can traverses an enclosed room while it does not know the map of the room at the beginning." << endl;
	cout << "	You can rewrite the set_environment() and go_and_clean::print_obstacle_route_and_car() to remove the API provided by opencv library." << endl;
	cout << "	Instead of presenting the DFS-cleaning process by video, you can cout the obstacle, cleaning status, car postion e.t.c.." << endl;
	cout << endl;
}
void set_environment()
{
	//read the imported environment file
	//if the imported environment file do not exist, use the default environment file
	char environment_file_name[80];
	char default_environment_file_name[] = "room1.png";
	char ch; int i;
	cout << endl;
	cout << "****************************************************************************************************" << endl;
	cout << "----------------------------------------------------------------------------------------------------" << endl;
	cout << "# IMPORT ENVIRONMENT IMAGE" << endl << endl;
	cout << "	Enter file name of the environment image." << endl;
	cout << "	(Draw the enclosed room and obstacles in it with black,";
	cout << "draw the start position with red." << endl;
	cout << "	You can turn frame an angle and draw your enclosed area to simulate a car with oblique start direction.)" << endl;
	cout << "	>> ";

	i = 0;
	while((ch = getchar()) != '\n')
	{
		if(ch == '.') break;
		environment_file_name[i] = ch;
		i++;
	}
	environment_file_name[i] = '.'; environment_file_name[i+1] = 'p'; environment_file_name[i+2] = 'n'; 
	environment_file_name[i+3] = 'g'; environment_file_name[i+4] = '\0'; 
	
	show_image = imread(environment_file_name, CV_LOAD_IMAGE_COLOR);
		
	if(! show_image.data )
	{
        cout << endl;
		cout << "	Environment image \"" << environment_file_name << "\" do not exist!" << endl ;
		cout << "	Use default environment image \"" << default_environment_file_name << "\"" << endl;
		show_image = imread(default_environment_file_name, CV_LOAD_IMAGE_COLOR);
		cout << endl;
	}
	else
	{
		cout << endl;
		cout << "	Environment image \"" << environment_file_name << "\" is successfully read-in!" << endl;
		cout << endl;
	}

	//after number of pixels per unit area given by user, resolution of the environment map is reduced from
	//(show_image.cols x show_image.rows) to (show_image.cols/scale x show_image.rows/scale)
	//informations in the image are then transformed and stored into the environment array 
	int unit_area = 0;
	cout << endl;
	cout << "****************************************************************************************************" << endl;
	cout << "----------------------------------------------------------------------------------------------------" << endl;
	cout << "# PIXEL PROCESS OF THE ENVIRONMENT IMAGE" << endl << endl;
	cout << "	You have imported an image which is with " << show_image.rows << " x " << show_image.cols << " pixels." << endl;
    cout << "	The program will now transforms it into an environment array." << endl;
	while(unit_area < 1)
	{
		cout << "	How many pixels do you assume a unit area to occupy? (must be larger than 1)" << endl;
		cout << "	>> ";
    	cin >> unit_area;
    	cout << endl;
	}
    scale = (int)sqrt(unit_area);

    resize(show_image,show_image,Size(show_image.cols+2*scale,show_image.rows+2*scale),0,0,CV_INTER_AREA);

	nr = show_image.rows/scale+1;
	nc = show_image.cols/scale+1;
	environment = new int*[nr];
    for(int i = 0; i < nr; i++)
    {
        environment[i] = new int[nc];
        for(int j = 0; j < nc; j++)
        {
            environment[i][j] = 0;
        }
    }

    //if any of the pixels in nrow = scale*i ~ scale*(i+1), ncol = scale*j ~ scale*(j+1) is black
	//(bgr.val[0] <= 50 && bgr.val[1] <= 50 && bgr.val[2] <= 50),
	//we will write 1 into environment[i][j], which is (i,j) exists obstacle
	//if any of the pixels in nrow = scale*i ~ scale*(i+1), ncol = scale*j ~ scale*(j+1) is red
	//(2*bgr.val[0] <= bgr.val[2] && 2*bgr.val[1] <= bgr.val[2]),
	//we will add i and j to at_environment_r and at_environment_c,respectively, and take the average, which is the start position of the car
	at_environment_r = 0;
	at_environment_c = 0;
    for( size_t nrow = 0; nrow < show_image.rows; nrow+=scale)
    {  
        for(size_t ncol = 0; ncol < show_image.cols; ncol+=scale)  
        {  
            bool flag = 0;
            for(int i = 0; i < scale; i++)
            {
                for(int j = 0; j < scale; j++)
                {
                    if(nrow+i >= 0 && nrow+i < show_image.rows && ncol+j >=0 && ncol+j < show_image.cols)
                    {
                        Vec3i bgr = show_image.at<Vec3b>(nrow+i,ncol+j);
                        if(bgr.val[0] <= 50 && bgr.val[1] <= 50 && bgr.val[2] <= 50)
                        {
                            flag = 1;
                        }
                        else if(2*bgr.val[0] <= bgr.val[2] && 2*bgr.val[1] <= bgr.val[2])
                        {
                            if(at_environment_r == 0 && at_environment_c == 0)
                            {
                                at_environment_r+=nrow/scale;
                                at_environment_c+=ncol/scale;
                            }
                            else
                            {
                                at_environment_r = (at_environment_r + nrow/scale)/2;
                                at_environment_c = (at_environment_c + ncol/scale)/2;
                            }
                        }
                    }
                }
            }

            if(flag == 1)
            {
                environment[nrow/scale][ncol/scale] = 1;
            }
        }
    }
	//setup animation parameters and name of the output animation files
	char global_animation_file_name[80];
	char local_animation_file_name[80];
	char default_global_animation_file_name[] = "DFS cleaning process_global.avi";
	char default_local_animation_file_name[] = "DFS cleaning process_local.avi";
	cout << endl;
	cout << "****************************************************************************************************" << endl;
	cout << "----------------------------------------------------------------------------------------------------" << endl;
	cout << "# EXPORT ANIMATION OF THE DFS CLEANING PROCESS" << endl << endl;
	cout << "	Name the animation file:" << endl;
	getchar();

	cout << "	>> ";
	i = 0;
	while((ch = getchar()) != '\n')
	{
		if(ch == '.') break;
		global_animation_file_name[i] = ch;
		local_animation_file_name[i] = ch;
		i++;
	}
	global_animation_file_name[i] = '_'; global_animation_file_name[i+1] = 'g'; global_animation_file_name[i+2] = 'l';
	global_animation_file_name[i+3] = 'o'; global_animation_file_name[i+4] = 'b'; global_animation_file_name[i+5] = 'a';
	global_animation_file_name[i+6] = 'l'; global_animation_file_name[i+7] = '.'; global_animation_file_name[i+8] = 'a';
	global_animation_file_name[i+9] = 'v'; global_animation_file_name[i+10] = 'i'; global_animation_file_name[i+11] = '\0';
	local_animation_file_name[i] = '_'; local_animation_file_name[i+1] = 'l'; local_animation_file_name[i+2] = 'o';
	local_animation_file_name[i+3] = 'c'; local_animation_file_name[i+4] = 'a'; local_animation_file_name[i+5] = 'l';
	local_animation_file_name[i+6] = '.'; local_animation_file_name[i+7] = 'a'; local_animation_file_name[i+8] = 'v';
	local_animation_file_name[i+9] = 'i'; local_animation_file_name[i+10] = '\0';

	global_animation_image = Mat(nr*scale, nc*scale, CV_8UC3, Scalar(255,255,255));
	Size S = Size(global_animation_image.cols, global_animation_image.rows);
	int codec = VideoWriter::fourcc('M','J','P','G');
	int FPS = 1000/10;
	if(i == 0)
	{
		cout << endl << "	Animation in global perspective: ";
		cout << default_global_animation_file_name << endl;
		global_output = VideoWriter(String(default_global_animation_file_name),codec,FPS,S,true);
	}
	else
	{
		cout << endl << "	Animation in global perspective: ";
		cout << global_animation_file_name << endl;
		global_output = VideoWriter(String(global_animation_file_name),codec,FPS,S,true);
	}
	local_animation_image = Mat(nr*scale, nc*scale, CV_8UC3, Scalar(0,0,0));
	if(i == 0)
	{
		cout << endl << "	Animation in robot's perspective: ";
		cout << default_local_animation_file_name << endl;
		local_output = VideoWriter(String(default_local_animation_file_name),codec,FPS,S,true);
	}
	else
	{
		cout << endl << "	Animation in robot's perspective: ";
		cout << local_animation_file_name << endl;
		local_output = VideoWriter(String(local_animation_file_name),codec,FPS,S,true);
	}

    //if (i,j) is in the nine-square division region centered on (at_environment_r,at_environment_c), 
	//set pixels in nrow = scale*i ~ scale*(i+1), ncol = scale*j ~ scale*(j+1) to blue
	//(bgr.val[0] = 255; bgr.val[1] = 0; bgr.val[2] = 0)
	//if environment[i][j] is 1, set pixels in nrow = scale*i ~ scale*(i+1), ncol = scale*j ~ scale*(j+1) to black
	//(bgr.val[0] = 0; bgr.val[1] = 0; bgr.val[2] = 0)
	for(int i = 0; i < nr; i++)
    {
        for(int j = 0; j < nc; j++)
        {
            if (i >= at_environment_r-1 && i <= at_environment_r+1 && j >= at_environment_c-1 && j <= at_environment_c+1)
			{
				for(size_t nrow = scale*i; nrow < scale*(i+1); nrow++)
				{
					for(size_t ncol = scale*j; ncol < scale*(j+1); ncol++)
					{
						Vec3b& bgr = global_animation_image.at<Vec3b>(nrow,ncol);
						bgr[0] = 255;
						bgr[1] = 0;
						bgr[2] = 0;									
					}
				}
            }
            else if(environment[i][j] == 1)
            {
                for(size_t nrow = i*scale; nrow < (i+1)*scale; nrow++)
				{
					for(size_t ncol = j*scale; ncol < (j+1)*scale; ncol++)
					{
						Vec3b& bgr = global_animation_image.at<Vec3b>(nrow,ncol);
						bgr[0] = 0;
						bgr[1] = 0;
						bgr[2] = 0;
					}
				}
            }
        }
    }

	//show image during execution
	namedWindow( "DFS cleaning process_global - Display window", CV_WINDOW_AUTOSIZE );
	namedWindow( "DFS cleaning process_local - Display window", CV_WINDOW_AUTOSIZE );
	resize(global_animation_image,show_image,Size(600*global_animation_image.cols/global_animation_image.rows,600));
    imshow( "DFS cleaning process_global - Display window", show_image ); 
    waitKey(5);
}

class go_and_clean
{
	//critical variables:
		//local_map: store for the already-detected local map
		//route: store for direction of the car during travel in order to recurse back
		//dir: store for the direction of the car
		//nr_size, nc_size: store for map size of the already-detected local map
		//at_local_nr, at_local_nc: store for the center postion of the car on local map
	node** local_map;
	stack<int> route;
	int dir, nr_size, nc_size, at_local_nr, at_local_nc;
public:
	go_and_clean()
	{
		//initialization:
			//3x5 surrounding is detected at the very beginning,
			//this 3x5 is see as the very first local map, that is,
			//size of the local map is initialized to be 3x5.
			//then, a 5x5 local map is set up to include the old 3x5 map into it,
			//size of the local map is increased to 5x5 thereafter.

			//center postion of the car starts from (2,2) on the local map,
			//direction of the car begins with 0, which is 0 degrees
		detect_surrounding();
		nr_size = 3; nc_size = 5;
		local_map = fill_map(surrounding, 5, 5, 0, 0);
		nr_size+=2;

		at_local_nr = 2; at_local_nc = 2;
		dir = 0;
	}

	void detect_surrounding()
	{
		//after setup a 3x5 nodes array,
		//set obstacle_or_not of the 3x2 nodes that car occupies to 0,
		//then, sensor will rotate from -PI/2 to PI/2 to detect obstacles in the environment while car do not turn,
		//update obstacle_or_not of the surrounding 9 nodes during detection
		surrounding = new node*[3];
		for (int i = 0; i < 3; i++)
		{
			surrounding[i] = new node[5];
		}
		
		for(int i = 0; i < 2; i++)
		{
			for(int j = 0; j < 3; j++)
			{
				surrounding[(2-1)+i][(2-1)+j].obstacle_or_not = 0;
			}
		}
		
		for (int i = 0; i <= 8; i++)
		{
			double theta = -PI/2 + PI/8 * i;
			double theta_dir = dir * 45.0 / 180.0 * PI;

			surrounding[2 - int(cos(theta)*2.85)][2 - int(sin(theta)*2.85)].obstacle_or_not \
			= environment[at_environment_r - int(cos(theta+theta_dir)*2.85)][at_environment_c - int(sin(theta+theta_dir)*2.85)];			
		}
	}
	
	void set_map_size(node**& _map, int _nr_size, int _nc_size)
	{
		//setup a new map with size of (_nr_size)x(_nc_size)
		_map = new node*[_nr_size];
		for (int i = 0; i< _nr_size; i++)
		{
			_map[i] = new node[_nc_size];
			for (int j = 0; j<_nc_size; j++)
			{
				_map[i][j].obstacle_or_not = 1;
			}
		}
	}

	node** fill_map(node**& old_map, int new_nr, int new_nc, int _px, int _py)
	{
		//setup a new map with size of (new_nr)x(new_nc),
		//fill the old map into the new map,
		//where old_map[0][0] is alligned with new_map[_px][_py]
		node** new_map;
		set_map_size(new_map, new_nr, new_nc);
		for (int i = 0; i<nr_size; i++)
		{
			for (int j = 0; j<nc_size; j++)
			{
				new_map[_px + i][_py + j] = old_map[i][j];
			}
		}
		return new_map;
	}

	void go_and_explore()
	{
		//while direction of the car is 0, 45, or 315 degrees,
		//and car is at the very top of the local map(at_local_nr = 1),
		//increase number of rows of the local map(nr_size++),
		//and fill old local map into new local map,
		//where local_map(old)[0][0] is alligned with local_map(new)[1][0]
		//after renewing the local map, position of the car on the local map is also renewed(at_local_nr++)
		if (dir % 8 == 0 || dir % 8 == 1 || dir % 8 == 7)
		{
			if (at_local_nr == 1)
			{
				local_map = fill_map(local_map, nr_size + 1, nc_size, 1, 0);
				nr_size++;
				at_local_nr++;
			}
		}
		//while direction of the car is 225, 270, or 315 degrees,
		//and car is at the very right of the local map(at_local_nc = nc_size-2),
		//increase number of columns of the local map(nc_size++),
		//and fill old local map into new local map,
		//where local_map(old)[0][0] is alligned with local_map(new)[0][0]
		//position of the car on the local map doesn't need to be renewed
		if (dir % 8 == 5 || dir % 8 == 6 || dir % 8 == 7)
		{
			if (at_local_nc == nc_size - 2)
			{
				local_map = fill_map(local_map, nr_size, nc_size + 1, 0, 0);
				nc_size++;
			}

		}
		//while direction of the car is 135, 180, or 225 degrees,
		//and car is at the very bottom of the local map(at_local_nr = nr_size-2),
		//increase number of rows of the local map(nr_size++),
		//and fill old local map into new local map,
		//where local_map(old)[0][0] is alligned with local_map(new)[0][0]
		//position of the car on the local map doesn't need to be renewed
		if (dir % 8 == 3 || dir % 8 == 4 || dir % 8 == 5)
		{
			if (at_local_nr == nr_size - 2)
			{
				local_map = fill_map(local_map, nr_size + 1, nc_size, 0, 0);
				nr_size++;
			}
		}
		//while direction of the car is 45, 90, or 135 degrees,
		//and car is at the very left of the local map(at_local_nc = 1),
		//increase number of columns of the local map(nc_size++),
		//and fill old local map into new local map,
		//where local_map(old)[0][0] is alligned with local_map(new)[0][1]
		//after renewing the local map, position of the car on the local map is also renewed(at_local_nc++)
		if (dir % 8 == 1 || dir % 8 == 2 || dir % 8 == 3)
		{
			if (at_local_nc == 1)
			{
				local_map = fill_map(local_map, nr_size, nc_size + 1, 0, 1);
				nc_size++;
				at_local_nc++;
			}
		}

	}

	void recurse()
	{		
		//based on the data structure of stack,
		//the last push back element of route will be the first to pop out
		//after pop out the direction of the car, we go for the opposite direction
		//as car moves, renew the postion of the car on local map,
		//renew the positon of the car in the environment as well

		//call explore_or_recurse at the end to determine to recurse or to explore
		switch (route.popout() % 8)
		{
		case 0:
			while (dir % 8 != 4)
				dir++;
			at_local_nr++;
			at_environment_r++;
			break;
		case 1:
			while (dir % 8 != 5)
				dir++;
			at_local_nr++;
			at_local_nc++;
			at_environment_r++;
			at_environment_c++;
			break;
		case 2:
			while (dir % 8 != 6)
				dir++;
			at_local_nc++;
			at_environment_c++;
			break;
		case 3:
			while (dir % 8 != 7)
				dir++;
			at_local_nr--;
			at_local_nc++;
			at_environment_r--;
			at_environment_c++;
			break;
		case 4:
			while (dir % 8 != 0)
				dir++;
			at_local_nr--;
			at_environment_r--;
			break;
		case 5:
			while (dir % 8 != 1)
				dir++;
			at_local_nr--;
			at_local_nc--;
			at_environment_r--;
			at_environment_c--;
			break;
		case 6:
			while (dir % 8 != 2)
				dir++;
			at_local_nc--;
			at_environment_c--;
			break;
		case 7:
			while (dir % 8 != 3)
				dir++;
			at_local_nr++;
			at_local_nc--;
			at_environment_r++;
			at_environment_c--;
			break;			
		}
		dir = dir - int(dir/8) * 8;

		explore_or_recurse();
	}
	void explore_or_recurse()
	{
		//check the front nodes are cleaned or not no matter the direction of the car,
		//if any of the front nodes is not cleaned yet, go forward to clean,
		//else do a virtual 45 degree CCW turn to see the next front nodes
		//if car has already done a virtual 360 turn(turn = 8) but all front nodes are either cleaned already or obstacles, 
		//enter recurse process if size of the route != 0, end cleaning if size of the route == 0
		
		//Last but not least, during go forward to clean, local map is expanded accordingly,
		//also, local map is renewed simultaneously by detecting the surrounding,
		//direction of the car is pushed back to route in order to recurse back later 
		for(int i = 0; i < 3; i++)
		{
			for(int j = 0; j < 3; j++)
			{
				local_map[(at_local_nr-1)+i][(at_local_nc-1)+j].obstacle_or_not = 0;
				local_map[(at_local_nr-1)+i][(at_local_nc-1)+j].cleaned_or_not += 1;
			}
		}
		print_obstacle_route_and_car();
		
		int turn = 0;
		while (turn<8)
		{		
			if ((dir + turn) % 8 == 0)
				if ((local_map[at_local_nr-2][at_local_nc].cleaned_or_not == 0 || local_map[at_local_nr-2][at_local_nc-1].cleaned_or_not == 0 || local_map[at_local_nr-2][at_local_nc+1].cleaned_or_not == 0)
				 && local_map[at_local_nr-2][at_local_nc].obstacle_or_not == 0 && local_map[at_local_nr-2][at_local_nc-1].obstacle_or_not == 0 && local_map[at_local_nr-2][at_local_nc+1].obstacle_or_not == 0)
				{
					while( dir % 8 != 0)
						dir++;
					turn = 0;
					at_local_nr--;
					at_environment_r--;
				}
				else
					turn++;
			else if ((dir + turn) % 8 == 1)
				if ((local_map[at_local_nr-2][at_local_nc-2].cleaned_or_not == 0 || local_map[at_local_nr-2][at_local_nc-1].cleaned_or_not == 0 || local_map[at_local_nr-2][at_local_nc].cleaned_or_not == 0 || local_map[at_local_nr-1][at_local_nc-2].cleaned_or_not == 0 || local_map[at_local_nr][at_local_nc-2].cleaned_or_not == 0)
				 && local_map[at_local_nr-2][at_local_nc-2].obstacle_or_not == 0 && local_map[at_local_nr-2][at_local_nc-1].obstacle_or_not == 0 && local_map[at_local_nr-2][at_local_nc].obstacle_or_not == 0 && local_map[at_local_nr-1][at_local_nc-2].obstacle_or_not == 0 && local_map[at_local_nr][at_local_nc-2].obstacle_or_not == 0)
				{
					while( dir % 8 != 1)
						dir++;
					turn = 0;
					at_local_nr--;
					at_local_nc--;
					at_environment_r--;
					at_environment_c--;
				}
				else
					turn++;
			else if ((dir + turn) % 8 == 2)
				if ((local_map[at_local_nr][at_local_nc-2].cleaned_or_not == 0 || local_map[at_local_nr-1][at_local_nc-2].cleaned_or_not == 0 || local_map[at_local_nr+1][at_local_nc-2].cleaned_or_not == 0)
				 && local_map[at_local_nr][at_local_nc-2].obstacle_or_not == 0 && local_map[at_local_nr-1][at_local_nc-2].obstacle_or_not == 0 && local_map[at_local_nr+1][at_local_nc-2].obstacle_or_not == 0)
				{
					while( dir % 8 != 2)
						dir++;
					turn = 0;
					at_local_nc--;
					at_environment_c--;
				}
				else
					turn++;
			else if ((dir + turn) % 8 == 3)
				if ((local_map[at_local_nr+2][at_local_nc-2].cleaned_or_not == 0 || local_map[at_local_nr+1][at_local_nc-2].cleaned_or_not == 0  || local_map[at_local_nr][at_local_nc-2].cleaned_or_not == 0 || local_map[at_local_nr+2][at_local_nc-1].cleaned_or_not == 0 || local_map[at_local_nr+2][at_local_nc].cleaned_or_not == 0)
				 && local_map[at_local_nr+2][at_local_nc-2].obstacle_or_not == 0 && local_map[at_local_nr+1][at_local_nc-2].obstacle_or_not == 0 && local_map[at_local_nr][at_local_nc-2].obstacle_or_not == 0 && local_map[at_local_nr+2][at_local_nc-1].obstacle_or_not == 0 && local_map[at_local_nr+2][at_local_nc].obstacle_or_not == 0)
				{
					while( dir % 8 != 3)
						dir++;
					turn = 0;
					at_local_nr++;
					at_local_nc--;
					at_environment_r++;
					at_environment_c--;
				}
				else
					turn++;
			else if ((dir + turn) % 8 == 4)
				if ((local_map[at_local_nr+2][at_local_nc].cleaned_or_not == 0 || local_map[at_local_nr+2][at_local_nc-1].cleaned_or_not == 0 || local_map[at_local_nr+2][at_local_nc+1].cleaned_or_not == 0)
				 && local_map[at_local_nr+2][at_local_nc].obstacle_or_not == 0 && local_map[at_local_nr+2][at_local_nc-1].obstacle_or_not == 0 && local_map[at_local_nr+2][at_local_nc+1].obstacle_or_not == 0)
				{
					while( dir % 8 != 4)
						dir++;
					turn = 0;
					at_local_nr++;
					at_environment_r++;
				}
				else
					turn++;
			else if ((dir + turn) % 8 == 5)
				if ((local_map[at_local_nr+2][at_local_nc+2].cleaned_or_not == 0 || local_map[at_local_nr+2][at_local_nc+1].cleaned_or_not == 0 || local_map[at_local_nr+2][at_local_nc].cleaned_or_not == 0 || local_map[at_local_nr+1][at_local_nc+2].cleaned_or_not == 0 || local_map[at_local_nr][at_local_nc+2].cleaned_or_not == 0)
				 && local_map[at_local_nr+2][at_local_nc+2].obstacle_or_not == 0 && local_map[at_local_nr+2][at_local_nc+1].obstacle_or_not == 0 && local_map[at_local_nr+2][at_local_nc].obstacle_or_not == 0 && local_map[at_local_nr+1][at_local_nc+2].obstacle_or_not == 0 && local_map[at_local_nr][at_local_nc+2].obstacle_or_not == 0)
				{
					while( dir % 8 != 5)
						dir++;
					turn = 0;
					at_local_nr++;
					at_local_nc++;
					at_environment_r++;
					at_environment_c++;
				}
				else
					turn++;
			else if ((dir + turn) % 8 == 6)
				if ((local_map[at_local_nr][at_local_nc+2].cleaned_or_not == 0 || local_map[at_local_nr+1][at_local_nc+2].cleaned_or_not == 0 || local_map[at_local_nr-1][at_local_nc+2].cleaned_or_not == 0)
				 && local_map[at_local_nr][at_local_nc+2].obstacle_or_not == 0 && local_map[at_local_nr+1][at_local_nc+2].obstacle_or_not == 0 && local_map[at_local_nr-1][at_local_nc+2].obstacle_or_not == 0)
				{
					while( dir % 8 != 6)
						dir++;
					turn = 0;
					at_local_nc++;
					at_environment_c++;
				}
				else
					turn++;
			else if ((dir + turn) % 8 == 7)
				if ((local_map[at_local_nr-2][at_local_nc+2].cleaned_or_not == 0 || local_map[at_local_nr-1][at_local_nc+2].cleaned_or_not == 0 || local_map[at_local_nr][at_local_nc+2].cleaned_or_not == 0 || local_map[at_local_nr-2][at_local_nc+1].cleaned_or_not == 0 || local_map[at_local_nr-2][at_local_nc].cleaned_or_not == 0)
				 && local_map[at_local_nr-2][at_local_nc+2].obstacle_or_not == 0 && local_map[at_local_nr-1][at_local_nc+2].obstacle_or_not == 0 && local_map[at_local_nr][at_local_nc+2].obstacle_or_not == 0 && local_map[at_local_nr-2][at_local_nc+1].obstacle_or_not == 0 && local_map[at_local_nr-2][at_local_nc].obstacle_or_not == 0)
				{
					while( dir % 8 != 7)
						dir++;
					turn = 0;
					at_local_nr--;
					at_local_nc++;
					at_environment_r--;
					at_environment_c++;
				}
				else
					turn++;
			
			if (turn == 0)
			{				
				go_and_explore();
				for(int i = 0; i < 3; i++)
				{
					for(int j = 0; j < 3; j++)
					{
						local_map[(at_local_nr-1)+i][(at_local_nc-1)+j].cleaned_or_not += 1;
					}
				}
				detect_surrounding();
				for (int i = 0; i <= 8; i++)
				{
					double theta = -PI/2 + PI/8 * i;
					double theta_dir = dir * 45.0 / 180.0 * PI;
					local_map[at_local_nr - int(cos(theta+theta_dir)*2.85)][at_local_nc - int(sin(theta+theta_dir)*2.85)].obstacle_or_not \
					= surrounding[2 - int(cos(theta)*2.85)][2 - int(sin(theta)*2.85)].obstacle_or_not;
				}
				route.pushback(dir);
				print_obstacle_route_and_car();
			}			
			dir = dir - int(dir/8) * 8;
		}
		if(route.size != 0)
			recurse();
	}


	void print_obstacle_route_and_car()
	{
		//if (i,j) is at the periphery of the local_map, set pixels in nrow = scale*i ~ scale*(i+1), ncol = scale*j ~ scale*(j+1) to red
		//(bgr.val[0] = 0; bgr.val[1] = 0; bgr.val[2] = 255)
		//if local_map[i][j].obstacle_or_not is 1, set pixels in nrow = scale*i ~ scale*(i+1), ncol = scale*j ~ scale*(j+1) to black
		//(bgr.val[0] = 0; bgr.val[1] = 0; bgr.val[2] = 0)
		//if (i,j) is in the nine-square division region centered on (at_local_nr,at_local_nc), 
		//set pixels in nrow = scale*i ~ scale*(i+1), ncol = scale*j ~ scale*(j+1) to blue
		//(bgr.val[0] = 255; bgr.val[1] = 0; bgr.val[2] = 0)
		//if local_map[i][j].cleaned_or_not > 0, set pixels in nrow = scale*i ~ scale*(i+1), ncol = scale*j ~ scale*(j+1) to different degrees of blue
		//(bgr.val[0] = 255; bgr.val[1] = 180 - 5*local_map[i][j].cleaned_or_not; bgr.val[2] = 180 - 5*local_map[i][j].cleaned_or_not)
		//else, set pixels to white(bgr.val[0] = 255; bgr.val[1] = 255; bgr.val[2] = 255)

		//note that, (at_environment_r-at_local_nr) and (at_environment_c-at_local_nc) are the vertical and horizontal offset between environment array and local_map array
		for (int i = 0; i < nr_size; i++)
		{
			for (int j = 0; j < nc_size; j++)
			{
				if(i == 0 || i == nr_size-1 || j == 0 || j == nc_size-1)
				{
					for(size_t nrow = scale*(at_environment_r-at_local_nr+i); nrow < scale*(at_environment_r-at_local_nr+i+1); nrow++)
					{
						for(size_t ncol = scale*(at_environment_c-at_local_nc+j); ncol < scale*(at_environment_c-at_local_nc+j+1); ncol++)
						{
							Vec3b& bgr = local_animation_image.at<Vec3b>(nrow,ncol);
							bgr[0] = 0;
							bgr[1] = 0;
							bgr[2] = 255;									
						}
					}
				}
				else if (local_map[i][j].obstacle_or_not == 1)
				{
					for(size_t nrow = scale*(at_environment_r-at_local_nr+i); nrow < scale*(at_environment_r-at_local_nr+i+1); nrow++)
					{
						for(size_t ncol = scale*(at_environment_c-at_local_nc+j); ncol < scale*(at_environment_c-at_local_nc+j+1); ncol++)
						{
							Vec3b& bgr = local_animation_image.at<Vec3b>(nrow,ncol);
							bgr[0] = 0;
							bgr[1] = 0;
							bgr[2] = 0;					
						}
					}
				}
				else
					if (i >= at_local_nr-1 && i <= at_local_nr+1 && j >= at_local_nc-1 && j <= at_local_nc+1)
					{
							for(size_t nrow = scale*(at_environment_r-at_local_nr+i); nrow < scale*(at_environment_r-at_local_nr+i+1); nrow++)
							{
								for(size_t ncol = scale*(at_environment_c-at_local_nc+j); ncol < scale*(at_environment_c-at_local_nc+j+1); ncol++)
								{
									Vec3b& bgr = global_animation_image.at<Vec3b>(nrow,ncol);
									bgr[0] = 255;
									bgr[1] = 0;
									bgr[2] = 0;									
								}
							}
							for(size_t nrow = scale*(at_environment_r-at_local_nr+i); nrow < scale*(at_environment_r-at_local_nr+i+1); nrow++)
							{
								for(size_t ncol = scale*(at_environment_c-at_local_nc+j); ncol < scale*(at_environment_c-at_local_nc+j+1); ncol++)
								{
									Vec3b& bgr = local_animation_image.at<Vec3b>(nrow,ncol);
									bgr[0] = 255;
									bgr[1] = 0;
									bgr[2] = 0;									
								}
							}
					}
					else
					{
						if (local_map[i][j].cleaned_or_not > 0)
						{
							for(size_t nrow = scale*(at_environment_r-at_local_nr+i); nrow < scale*(at_environment_r-at_local_nr+i+1); nrow++)
							{
								for(size_t ncol = scale*(at_environment_c-at_local_nc+j); ncol < scale*(at_environment_c-at_local_nc+j+1); ncol++)
								{
									Vec3b& bgr = global_animation_image.at<Vec3b>(nrow,ncol);
									bgr[0]=255;
									bgr[1]=180-5*local_map[i][j].cleaned_or_not;
									bgr[2]=180-5*local_map[i][j].cleaned_or_not;									
								}
							}
							for(size_t nrow = scale*(at_environment_r-at_local_nr+i); nrow < scale*(at_environment_r-at_local_nr+i+1); nrow++)
							{
								for(size_t ncol = scale*(at_environment_c-at_local_nc+j); ncol < scale*(at_environment_c-at_local_nc+j+1); ncol++)
								{
									Vec3b& bgr = local_animation_image.at<Vec3b>(nrow,ncol);
									bgr[0]=255;
									bgr[1]=180-5*local_map[i][j].cleaned_or_not;
									bgr[2]=180-5*local_map[i][j].cleaned_or_not;									
								}
							}
						}
						else
						{
							for(size_t nrow = scale*(at_environment_r-at_local_nr+i); nrow < scale*(at_environment_r-at_local_nr+i+1); nrow++)
							{
								for(size_t ncol = scale*(at_environment_c-at_local_nc+j); ncol < scale*(at_environment_c-at_local_nc+j+1); ncol++)
								{
									Vec3b& bgr = global_animation_image.at<Vec3b>(nrow,ncol);
									bgr[0] = 255;
									bgr[1] = 255;
									bgr[2] = 255;
								}
							}
							for(size_t nrow = scale*(at_environment_r-at_local_nr+i); nrow < scale*(at_environment_r-at_local_nr+i+1); nrow++)
							{
								for(size_t ncol = scale*(at_environment_c-at_local_nc+j); ncol < scale*(at_environment_c-at_local_nc+j+1); ncol++)
								{
									Vec3b& bgr = local_animation_image.at<Vec3b>(nrow,ncol);
									bgr[0] = 255;
									bgr[1] = 255;
									bgr[2] = 255;									
								}
							}
						}
					}		
			}
		}
		//output the images to animation
		global_output << global_animation_image;
		local_output << local_animation_image;
		//show images during execution
		resize(global_animation_image,show_image,Size(600*global_animation_image.cols/global_animation_image.rows,600));
    	imshow( "DFS cleaning process_global - Display window", show_image );
		resize(local_animation_image,show_image,Size(600*local_animation_image.cols/local_animation_image.rows,600));
    	imshow( "DFS cleaning process_local - Display window", show_image ); 
        waitKey(5);
	}
};
int main()
{
	instruction();
	set_environment();
	Sleep(3000);
	go_and_clean cleaner;
	cleaner.explore_or_recurse();

	return 0;
}
