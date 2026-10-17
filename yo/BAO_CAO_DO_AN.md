# BÁO CÁO ĐỒ ÁN MÔN HỌC: CẤU TRÚC DỮ LIỆU VÀ GIẢI THUẬT
## ĐỀ TÀI: HỆ THỐNG QUẢN LÝ THƯ VIỆN & QUAN HỆ NGƯỜI MƯỢN SÁCH

---

- **Môn học:** Cấu trúc Dữ liệu và Giải thuật (Data Structures and Algorithms - DSA)
- **Ngôn ngữ lập trình:** C++ (Chuẩn C++11 trở lên)
- **Mô hình nghiệp vụ:** Quan hệ 1 &ndash; N (Một đầu sách &ndash; Nhiều sinh viên mượn cho đến khi hết số lượng còn lại)
- **Quy chuẩn kỹ thuật:** **100% Cài đặt thủ công bằng Con trỏ và Mảng**, tuyệt đối **KHÔNG** sử dụng thư viện mẫu STL (`<vector>`, `<stack>`, `<queue>`, `<list>`, `<algorithm>`, `std::sort`).

---

## 1. MÔ HÌNH BÀI TOÁN & QUAN HỆ SÁCH - NGƯỜI MƯỢN

### 1.1. Bối cảnh thực tế
Trong thực tế, mỗi đầu sách trong thư viện luôn có tổng số lượng (`tongSoLuong`) nhất định và có thể cho nhiều sinh viên mượn đồng thời. Mỗi khi có sinh viên mượn một cuốn sách:
- Số lượng sách còn lại trong kho giảm đi (`soLuongConLai--`).
- Một phiếu mượn được lập ghi lại đầy đủ: **Họ tên sinh viên**, **Mã số sinh viên (MSSV)**, **Mã sách**, **Tên sách**, **Loại sách** và **Ngày mượn**.
- Khi `soLuongConLai == 0`, sách chính thức hết trong kho. Sinh viên đến sau sẽ được xếp vào **Hàng đợi đặt trước (Queue FIFO)**.
- Khi một sinh viên trả sách, `soLuongConLai++`, phiếu mượn được hủy và hệ thống ưu tiên phục vụ cho người đang đứng đầu Hàng đợi.

---

## 2. CÁC CẤU TRÚC DỮ LIỆU TỰ THIẾT KẾ

### 2.1. Cấu trúc Đầu Sách (`struct Book`)
```cpp
struct Book {
    string maSach;          // Mã sách duy nhất (Primary Key), ví dụ: "CS101"
    string tenSach;         // Tên cuốn sách
    string loaiSach;        // Thể loại: "Giáo trình", "Kỹ thuật", "Tham khảo", "Khoa học"...
    int tongSoLuong;        // Tổng số lượng sách thư viện sở hữu
    int soLuongConLai;      // Số lượng sách hiện còn trong kho (cho phép mượn nếu > 0)
    int luotMuon;           // Tổng số lần sách đã được mượn
};

struct NodeSach {
    Book data;
    NodeSach* next;         // Con trỏ liên kết danh sách các đầu sách
};
```

### 2.2. Cấu trúc Phiếu Mượn Sinh Viên (`struct PhieuMuon`)
```cpp
struct PhieuMuon {
    string maPhieu;         // Mã phiếu mượn tự động: "PM001", "PM002"...
    string tenSinhVien;     // Họ và tên sinh viên mượn sách
    string maSinhVien;      // Mã số sinh viên (MSSV)
    string maSach;          // Mã sách mượn
    string tenSach;         // Tên cuốn sách mượn
    string loaiSach;        // Thể loại cuốn sách
    string ngayMuon;        // Ngày lập phiếu mượn
};

struct NodePhieuMuon {
    PhieuMuon data;
    NodePhieuMuon* next;    // Con trỏ liên kết danh sách các phiếu mượn
};
```

### 2.3. Ngăn xếp Hoàn tác (`struct Stack` LIFO)
```cpp
enum ActionType { ACTION_MUON_SACH, ACTION_XOA_SACH };

struct UndoAction {
    ActionType loaiHanhDong;
    Book sach;              // Thông tin sách
    PhieuMuon phieu;        // Thông tin phiếu mượn
};

struct StackNode {
    UndoAction data;
    StackNode* next;
};

struct Stack {
    StackNode* top;         // Con trỏ đỉnh ngăn xếp LIFO
    int count;
};
```

### 2.4. Hàng đợi Đặt trước (`struct Queue` FIFO)
```cpp
struct PhieuDatCho {
    string tenSinhVien;     // Họ tên sinh viên chờ mượn
    string maSinhVien;      // MSSV
    string maSachCanMuon;   // Mã sách cần mượn
};

struct QueueNode {
    PhieuDatCho data;
    QueueNode* next;
};

struct Queue {
    QueueNode* front;       // Đầu hàng đợi
    QueueNode* rear;        // Cuối hàng đợi
    int count;
};
```

---

## 3. NGUYÊN LÝ HOẠT ĐỘNG CỦA CÁC THUẬT TOÁN

### 3.1. Thuật toán Mượn Sách & Quản lý Số lượng Kho
- **Bước 1:** Tìm cuốn sách theo mã bằng thuật toán **Binary Search $O(\log n)$**.
- **Bước 2:** Kiểm tra điều kiện số lượng:
  - Nếu `soLuongConLai > 0`: Cho mượn thành công. Giảm `soLuongConLai--`, tăng `luotMuon++`. Nhập Họ tên SV và MSSV, tạo `PhieuMuon` thêm vào danh sách liên kết phiếu mượn. Lưu vào Stack Undo.
  - Nếu `soLuongConLai == 0`: Sách đã hết. Mời sinh viên đăng ký xếp vào Hàng đợi đặt trước (Queue FIFO).

### 3.2. Thuật toán Trả Sách & Phục vụ Hàng đợi
- **Bước 1:** Nhập MSSV và Mã sách cần trả. Duyệt danh sách liên kết phiếu mượn để xóa phiếu.
- **Bước 2:** Tăng lại số lượng sách trong kho: `soLuongConLai++`.
- **Bước 3:** Kiểm tra `Queue`: Nếu có sinh viên đang đợi sách này $\rightarrow$ thông báo ưu tiên phục vụ.

### 3.3. Thuật toán Hoàn tác (Undo) bằng Stack LIFO
- Khi Undo thao tác mượn sách: Tự động rút hành động gần nhất trên đỉnh Stack ra (`pop`), hủy phiếu mượn khỏi danh sách phiếu mượn, khôi phục lại `soLuongConLai++` và giảm `luotMuon--`.

### 3.4. Thuật toán QuickSort & Binary Search
- Chuyển toàn bộ sách sang mảng động `Book* arr = new Book[n]`.
- Chạy thuật toán QuickSort (phân hoạch Lomuto) sắp xếp theo mã sách với chi phí $O(n \log n)$.
- Thực hiện Binary Search với độ phức tạp $O(\log n)$.

---

## 4. BẢNG TỔNG HỢP ĐỘ PHỨC TẠP THUẬT TOÁN

| Thao tác / Chức năng | Cấu trúc dữ liệu | Độ phức tạp Thời gian | Độ phức tạp Không gian |
| :--- | :--- | :--- | :--- |
| Thêm đầu sách mới | Singly Linked List (Sách) | $O(n)$ | $O(1)$ |
| Cho sinh viên mượn sách | Singly Linked List (Phiếu mượn) | $O(n)$ | $O(1)$ |
| Nhận trả sách từ sinh viên | Singly Linked List (Phiếu mượn) | $O(n)$ | $O(1)$ |
| Lưu thao tác vào Undo | Stack (Linked List) | $O(1)$ | $O(1)$ |
| Rút thao tác Hoàn tác | Stack (Linked List) | $O(1)$ | $O(1)$ |
| Đặt chỗ khi hết sách | Queue (con trỏ rear) | $O(1)$ | $O(1)$ |
| Phục vụ đặt chỗ (Dequeue) | Queue (con trỏ front) | $O(1)$ | $O(1)$ |
| Sắp xếp danh mục sách | Mảng động + QuickSort | Trung bình: $O(n \log n)$ | $O(\log n)$ |
| Tìm kiếm mã sách | Mảng động + Binary Search | $O(\log n)$ | $O(1)$ |
| Xuất Dashboard Web | File Stream + Linked Lists | $O(n + m)$ | $O(1)$ |

---

## 5. HỎI ĐÁP BẢO VỆ ĐỒ ÁN TRƯỚC HỘI ĐỒNG

- **Câu 1: Em cài đặt quan hệ giữa Sách và Người mượn như thế nào?**  
  *Trả lời:* Em tổ chức theo mô hình quan hệ 1-N. Mỗi đầu sách có tổng số lượng và số lượng còn lại. Một đầu sách có thể cho nhiều sinh viên mượn thông qua danh sách các phiếu mượn (`PhieuMuon`: họ tên, MSSV, mã sách, loại sách, ngày mượn) cho đến khi số lượng còn lại trong kho bằng 0.

- **Câu 2: Tại sao em không dùng mảng tĩnh `PhieuMuon ds[1000]`?**  
  *Trả lời:* Vì số lượng sinh viên mượn sách biến động liên tục. Danh sách liên kết đơn giúp cấp phát động từng nút khi mượn và giải phóng ngay khi trả sách, tối ưu hóa bộ nhớ và không sợ tràn mảng.
