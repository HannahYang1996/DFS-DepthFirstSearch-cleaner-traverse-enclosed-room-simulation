# README  
## 深度優先探索機器人探索及遍歷封閉空間之模擬  

### 專案簡介  
本專案模擬一機器人在對環境完全未知的情況下，逐步探索出環境中的有效行走範圍，其中是依據深度優先探索策略完成空間的遍歷。使用者可以匯入任何封閉空間地圖，而程式將對封閉空間做出深度優先探索遍歷，並將此過程輸出成影片。  

### 安裝說明  
1. 使用函式庫：
* opencv 3.4.14  

2. 編譯檔案：  
```shell
g++ -o DFS-cleaner DFS-cleaner.cpp -IC:/opencv/build/include  -LC:/opencv/opencv_mingw64_build/lib -llibopencv_core3414 -llibopencv_highgui3414 -llibopencv_imgproc3414 -llibopencv_imgcodecs3414 -llibopencv_videoio3414  
```
3. 執行：
```shell
DFS-cleaner.exe
```
### 使用說明  
1. 使用者需要先使用小畫家或其他繪圖軟體繪製出一**封閉空間**，以及其中的障礙物，並以 .**png** 檔案格式儲存
* 以**黑筆**繪製**障礙物及牆面**
* 以**紅筆**繪製**機器人起始位置**

下圖即為一封閉空間圖片的繪製範例，圖片解析度為 1000 x 2000 像素  

![](https://i.imgur.com/WVs8T9v.png)

2. 匯入**封閉空間的圖片**後，程式會告訴使用者該圖片的解析度為何，並詢問使用者要**將空間中最小單位面積當作多少像素**，依據使用者之要求，空間地圖將作出相應的像素處理。
下面三圖是將上圖做不同像素處理後之結果：  

* 將最小單位面積當作 10 像素做處理之結果  

![](https://i.imgur.com/OzqtxZl.png)  

* 將最小單位面積當作 200 像素做處理之結果  

![](https://i.imgur.com/gpmeSSR.png)  

* 將最小單位面積當作 1000 像素做處理之結果  

![](https://i.imgur.com/vuOgNGc.png)  

3. 緊接著，程式會對做完像素處理的封閉空間進行**深度優先探索遍歷**，透過使用opencv提供的API，**成果將以.avi之影片格式輸出**。其中包含：
* **影片一：已知空間地圖的人之視角**

![](https://i.imgur.com/Y8cyNJn.gif)

* **影片二：未知空間地圖的機器人之視角**

  影片中，紅色方框為機器人管理的空間地圖目前的大小，空間地圖中(紅色方框內)，被機器人視為障礙物的部分標示為黑色；藍色方塊為機器人本身；淺藍色部分為已經行經的區域，並且顏色越深表示經過越多次；白色部分為尚未前往之區域

![](https://i.imgur.com/veTSYTq.gif)

### 註記
機器人在圖面上的起始方向始終朝向上，因此若要表示**機器人在空間中的起始方向為斜向**，使用者可以**將空間地圖旋轉一角度後繪製**。
* 轉向繪製的空間地圖：

![](https://i.imgur.com/7n5EPzi.png)

* 深度優先探索遍歷之成果：

![](https://i.imgur.com/T3bu8i9.gif)
![](https://i.imgur.com/UkKUEqK.gif)
