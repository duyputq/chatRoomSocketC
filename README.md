# ChatRoom trong ngôn ngữ C

## Mô tả
Tạo một phòng chat cho các client có thể gửi nhận message cho nhau dưa trên lập trình Socket và đa luồng

## Cách chạy

### Bước 1: Clone với HTTPS
`git clone https://github.com/duyputq/chatRoomSocketC.git`

### Bước 2: Biên dịch

`gcc serverChat.c -o server`
`gcc clientChat.c -o client`

### Bước 3: Chạy chương trình
chạy server: 
`./server <port>` (ví dụ `./server 4444`) 

chạy nhiều client trên terminal khác nhau:
`./client <port>`

### Bước 4: Chat
nhập message và message sẽ được gửi qua các client và server 

## Hình ảnh minh họa
![ChatRoom](https://github.com/duyputq/chatRoomSocketC/assets/100561979/b1366eef-3766-4fad-8378-0a7039bc5616)
