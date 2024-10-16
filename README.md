# FHESearch

在我們的 FHE 程式中，分成使用者端與伺服器端。
使用者端可以供使用者產生金鑰、加密與解密；
而搜尋的功能則由伺服器端執行。

# 安裝

首先使用者與伺服器都需要安裝 OpenFHE，
並且在各自的 **build** 資料夾下安裝 Node.js 與所需的 modules。
並且執行 make 編譯程式。

# 啟動服務 

## 使用者端
使用者需要開啟中端機， 切換至路徑 FHE_client/build/ 下，執行指令

  npm start 

以啟動 Electron 視窗，在視窗中會有各個功能的按鈕，請依序執行。

## 伺服器端
切換至路徑 FHE_server/build/ 下，執行指令

  node server.js

# 執行

## 產生金鑰
首先需先選擇金鑰的安全性，有 TOY 和 128-bit 兩種，
選擇安全性後點擊 Keygen 產生金鑰，金鑰產生後需設定密碼以保護私鑰。

## 加密
請輸入要搜尋的 Query，
例如 「**ID = 1**」 、 「**Name = John AND ID = 3**」 等等；
並留意每個符號中間都需有一個空格分開；
完成加密後會產生 required.zip 檔案。 

## 搜尋
請先確認伺服器端已經啟動服務（執行 node server.js 指令）
連線至網頁（在此以 140.122.185.199:8080 為例）
成功連線後會看到登入介面，請輸入帳號及密碼登入。
登入後請依序上傳
1. 欲搜尋的 csv 檔案
2. required.zip
在下方選單中搜尋方才上傳的 csv 檔案，
最後點擊搜尋，伺服器端便會開始執行搜尋的程式。

## 解密
搜尋完成後，請從網頁上下載 **result** 檔案，
將 result 放入使用者電腦上的 FHE_client/build/Data/ 資料夾裡；
回到 Electron 視窗，點擊 Decrypt 解密；
這時需要輸入在產生金鑰時設定的密碼，以解鎖私鑰；並且需要重新設定密碼保護金鑰。
最後顯示解密的結果，為 csv 檔案中，符合 Query 的資料筆數。
