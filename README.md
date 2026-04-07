# Space Odyssey

## 1. Hướng dẫn Build

Tải và cài đặt w64devkit tại: https://github.com/skeeto/w64devkit/releases
Tải à cài đặt cmake tại: https://cmake.org/download/

Sau khi tải, mở w64devkit.exe lên và chọn thư mục cần tải vào (ví dụ: C:\w64devkit), cmake tương tự

Kiểm tra bằng cách mở terminal lên gõ:
```
cmake -v
gcc -v
g++ -v
```
Clone repository về
```
git clone https://github.com/trcminh/space_odyssey.git
```
Build và chạy 
```
cmake -G "Unix Makefiles" -B build -S .
cmake --build build
```


