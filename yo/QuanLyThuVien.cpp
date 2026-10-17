/*
========================================================================================
     DO AN MON HOC: CAU TRUC DU LIEU VA GIAI THUAT (DATA STRUCTURES & ALGORITHMS)
     DE TAI: HE THONG QUAN LY THU VIEN (LIBRARY MANAGEMENT SYSTEM)
========================================================================================
 Tinh nang nang cap:
  1. Quan ly Sach: maSach, tenSach, loaiSach, tongSoLuong, soLuongConLai, luotMuon.
     Mot dau sach co the cho nhieu sinh vien muon den khi soLuongConLai == 0.
  2. Quan he Nguoi muon (Phieu muon): hoTen, maSinhVien, maSach, tenSach, loaiSach, ngayMuon.
     Quan ly bang Danh sach lien ket don cac Phieu muon.
  3. Hang doi Dat cho (Queue FIFO): Khi sach het (soLuongConLai == 0), sinh vien dang ky
     duoc xep vao Hang doi cho voi thong tin hoTen, maSinhVien, maSach.
  4. Ngan xep Hoan tac (Stack LIFO): Tu cai dat bang DSLK de Undo thao tac Muon hoac Xoa.
  5. Tim kiem nhanh: Chuyen sang Mang dong + QuickSort + Binary Search O(log n).
  6. Luu tru ben vung: data.txt (sach) va phieu_muon.txt (nguoi muon).
  7. Xuat Web Dashboard: File thong_ke.html giao dien sang (Light Theme) chuyen nghiep.

 CAM KET: 100% KHONG SU DUNG THU VIEN STL (<vector>, <stack>, <queue>, <algorithm>, std::sort).
========================================================================================
*/

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

// =====================================================================================
// PHAN 1: KHAI BAO CAC CAU TRUC DU LIEU (DATA STRUCTURES)
// =====================================================================================

// 1.1. Cau truc thong tin mot cuon sach trong kho
struct Book {
    string maSach;          // Ma sach duy nhat (vi du: "CS101")
    string tenSach;         // Ten cuon sach
    string loaiSach;        // The loai: "Giao trinh", "Ky thuat", "Tham khao", "Khoa hoc"...
    int tongSoLuong;        // Tong so luong sach thu vien so huu
    int soLuongConLai;      // So luong sach hien con trong kho (cho phep muon neu > 0)
    int luotMuon;           // Tong so lan sach da duoc muon tu truoc den nay
};

// 1.2. Node trong Danh sach lien ket don quan ly Sach
struct NodeSach {
    Book data;
    NodeSach* next;
};

// 1.3. Cau truc Thong tin Nguoi muon sach (Phieu muon)
struct PhieuMuon {
    string maPhieu;         // Ma phieu muon tu dong: "PM001", "PM002"...
    string tenSinhVien;     // Ho va ten sinh vien muon sach
    string maSinhVien;      // Ma so sinh vien (MSSV)
    string maSach;          // Ma sach muon
    string tenSach;         // Ten sach muon
    string loaiSach;        // The loai sach
    string ngayMuon;        // Ngay muon (vi du: "16/10/2026")
};

// 1.4. Node trong Danh sach lien ket don quan ly Phieu muon
struct NodePhieuMuon {
    PhieuMuon data;
    NodePhieuMuon* next;
};

// 1.5. Cau truc phuc vu chuc nang Hoan tac (Undo) bang Stack
enum ActionType {
    ACTION_MUON_SACH,       // Sinh vien vua muon sach -> Undo se huy phieu va tra lai so luong
    ACTION_XOA_SACH         // Thu thu vua xoa sach -> Undo se them lai sach vao kho
};

struct UndoAction {
    ActionType loaiHanhDong;
    Book sach;              // Thong tin sach lien quan
    PhieuMuon phieu;        // Thong tin phieu muon lien quan (neu la muon sach)
};

struct StackNode {
    UndoAction data;
    StackNode* next;
};

struct Stack {
    StackNode* top;         // Con tro dinh Stack
    int count;              // So luong hanh dong co the Undo
};

// 1.6. Cau truc phuc vu Hang doi Dat truoc sach (Queue FIFO)
struct PhieuDatCho {
    string tenSinhVien;     // Ho ten sinh vien dat cho
    string maSinhVien;      // MSSV
    string maSachCanMuon;   // Ma sach muon muon khi co nguoi tra
};

struct QueueNode {
    PhieuDatCho data;
    QueueNode* next;
};

struct Queue {
    QueueNode* front;       // Con tro dau Hang doi
    QueueNode* rear;        // Con tro cuoi Hang doi
    int count;              // So nguoi dang xep hang
};

// =====================================================================================
// PHAN 2: CAI DAT NGAN XEP (STACK) - PHUC VU HOAN TAC (UNDO)
// =====================================================================================

void initStack(Stack& s) {
    s.top = NULL;
    s.count = 0;
}

bool isStackEmpty(const Stack& s) {
    return s.top == NULL;
}

void push(Stack& s, UndoAction action) {
    StackNode* newNode = new StackNode;
    newNode->data = action;
    newNode->next = s.top;
    s.top = newNode;
    s.count++;
}

bool pop(Stack& s, UndoAction& action) {
    if (isStackEmpty(s)) return false;
    StackNode* temp = s.top;
    action = temp->data;
    s.top = s.top->next;
    delete temp;
    s.count--;
    return true;
}

void clearStack(Stack& s) {
    UndoAction dummy;
    while (pop(s, dummy)) {}
}

// =====================================================================================
// PHAN 3: CAI DAT HANG DOI (QUEUE) - PHUC VU DAT CHO SACH (FIFO)
// =====================================================================================

void initQueue(Queue& q) {
    q.front = NULL;
    q.rear = NULL;
    q.count = 0;
}

bool isQueueEmpty(const Queue& q) {
    return q.front == NULL;
}

void enqueue(Queue& q, PhieuDatCho phieu) {
    QueueNode* newNode = new QueueNode;
    newNode->data = phieu;
    newNode->next = NULL;

    if (isQueueEmpty(q)) {
        q.front = newNode;
        q.rear = newNode;
    } else {
        q.rear->next = newNode;
        q.rear = newNode;
    }
    q.count++;
}

bool dequeue(Queue& q, PhieuDatCho& phieu) {
    if (isQueueEmpty(q)) return false;
    QueueNode* temp = q.front;
    phieu = temp->data;
    q.front = q.front->next;

    if (q.front == NULL) {
        q.rear = NULL;
    }
    delete temp;
    q.count--;
    return true;
}

void inHangDoiConsole(const Queue& q) {
    if (isQueueEmpty(q)) {
        cout << "  -> Hang doi dat cho hien dang trong!\n";
        return;
    }
    cout << "\n========================================================================================\n";
    cout << "                   DANH SACH SINH VIEN DANG XEP HANG DAT TRUOC (FIFO)                   \n";
    cout << "========================================================================================\n";
    cout << left << setw(6)  << "STT"
         << setw(16) << "MSSV"
         << setw(30) << "Ho va Ten Sinh Vien"
         << setw(15) << "Ma Sach Cho" << "\n";
    cout << "----------------------------------------------------------------------------------------\n";

    QueueNode* cur = q.front;
    int stt = 1;
    while (cur != NULL) {
        cout << left << setw(6)  << stt++
             << setw(16) << cur->data.maSinhVien
             << setw(30) << cur->data.tenSinhVien
             << setw(15) << cur->data.maSachCanMuon << "\n";
        cur = cur->next;
    }
    cout << "========================================================================================\n";
    cout << "Tong so sinh vien dang xep hang cho: " << q.count << "\n";
}

void clearQueue(Queue& q) {
    PhieuDatCho dummy;
    while (dequeue(q, dummy)) {}
}

// =====================================================================================
// PHAN 4: DANH SACH LIEN KET DON QUAN LY SACH (BOOK LIST)
// =====================================================================================

NodeSach* createNodeSach(Book b) {
    NodeSach* newNode = new NodeSach;
    newNode->data = b;
    newNode->next = NULL;
    return newNode;
}

void themSachVaoCuoi(NodeSach*& head, Book b) {
    NodeSach* newNode = createNodeSach(b);
    if (head == NULL) {
        head = newNode;
        return;
    }
    NodeSach* cur = head;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = newNode;
}

int demSoLuongDauSach(NodeSach* head) {
    int count = 0;
    NodeSach* cur = head;
    while (cur != NULL) {
        count++;
        cur = cur->next;
    }
    return count;
}

NodeSach* timSachTheoMa(NodeSach* head, string maSach) {
    NodeSach* cur = head;
    while (cur != NULL) {
        if (cur->data.maSach == maSach) return cur;
        cur = cur->next;
    }
    return NULL;
}

bool xoaSachTheoMa(NodeSach*& head, string maSach, Book& sachBiXoa) {
    if (head == NULL) return false;

    // Xoa phan tu dau tien (head)
    if (head->data.maSach == maSach) {
        NodeSach* temp = head;
        sachBiXoa = head->data;
        head = head->next;
        delete temp;
        return true;
    }

    // Xoa phan tu tu vi tri thu 2 tro di
    NodeSach* prev = head;
    NodeSach* cur = head->next;
    while (cur != NULL) {
        if (cur->data.maSach == maSach) {
            sachBiXoa = cur->data;
            prev->next = cur->next;
            delete cur;
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    return false;
}

void inDanhSachSachConsole(NodeSach* head) {
    if (head == NULL) {
        cout << "\n  [!] Hien tai thu vien chua co cuon sach nao!\n";
        return;
    }

    cout << "\n====================================================================================================\n";
    cout << "                                     DANH MUC SACH TRONG THU VIEN                                   \n";
    cout << "====================================================================================================\n";
    cout << left << setw(10) << "Ma Sach"
         << setw(36) << "Ten Sach"
         << setw(16) << "Loai Sach"
         << setw(10) << "Tong So"
         << setw(12) << "Con Lai"
         << setw(14) << "Luot Muon"
         << setw(14) << "Tinh Trang" << "\n";
    cout << "----------------------------------------------------------------------------------------------------\n";

    NodeSach* cur = head;
    while (cur != NULL) {
        string tinhTrangStr = (cur->data.soLuongConLai > 0) ? "[Con sach]" : "[Het sach]";
        cout << left << setw(10) << cur->data.maSach
             << setw(36) << cur->data.tenSach
             << setw(16) << cur->data.loaiSach
             << setw(10) << cur->data.tongSoLuong
             << setw(12) << cur->data.soLuongConLai
             << setw(14) << cur->data.luotMuon
             << setw(14) << tinhTrangStr << "\n";
        cur = cur->next;
    }
    cout << "====================================================================================================\n";
    cout << "Tong cong: " << demSoLuongDauSach(head) << " dau sach trong kho.\n";
}

void clearListSach(NodeSach*& head) {
    while (head != NULL) {
        NodeSach* temp = head;
        head = head->next;
        delete temp;
    }
}

// =====================================================================================
// PHAN 5: DANH SACH LIEN KET DON QUAN LY PHIEU MUON (BORROWER LIST)
// =====================================================================================

NodePhieuMuon* createNodePhieuMuon(PhieuMuon pm) {
    NodePhieuMuon* newNode = new NodePhieuMuon;
    newNode->data = pm;
    newNode->next = NULL;
    return newNode;
}

void themPhieuMuon(NodePhieuMuon*& head, PhieuMuon pm) {
    NodePhieuMuon* newNode = createNodePhieuMuon(pm);
    if (head == NULL) {
        head = newNode;
        return;
    }
    NodePhieuMuon* cur = head;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = newNode;
}

bool xoaPhieuMuon(NodePhieuMuon*& head, string maSV, string maSach, PhieuMuon& phieuXoa) {
    if (head == NULL) return false;

    if (head->data.maSinhVien == maSV && head->data.maSach == maSach) {
        NodePhieuMuon* temp = head;
        phieuXoa = head->data;
        head = head->next;
        delete temp;
        return true;
    }

    NodePhieuMuon* prev = head;
    NodePhieuMuon* cur = head->next;
    while (cur != NULL) {
        if (cur->data.maSinhVien == maSV && cur->data.maSach == maSach) {
            phieuXoa = cur->data;
            prev->next = cur->next;
            delete cur;
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    return false;
}

int demSoPhieuMuon(NodePhieuMuon* head) {
    int count = 0;
    NodePhieuMuon* cur = head;
    while (cur != NULL) {
        count++;
        cur = cur->next;
    }
    return count;
}

void inDanhSachPhieuMuonConsole(NodePhieuMuon* head) {
    if (head == NULL) {
        cout << "\n  [*] Hien tai chua co sinh vien nao muon sach!\n";
        return;
    }

    cout << "\n====================================================================================================\n";
    cout << "                             DANH SACH SINH VIEN DANG MUON SACH                                     \n";
    cout << "====================================================================================================\n";
    cout << left << setw(10) << "Ma Phieu"
         << setw(14) << "MSSV"
         << setw(26) << "Ho va Ten Sinh Vien"
         << setw(10) << "Ma Sach"
         << setw(32) << "Ten Cuon Sach"
         << setw(14) << "Ngay Muon" << "\n";
    cout << "----------------------------------------------------------------------------------------------------\n";

    NodePhieuMuon* cur = head;
    while (cur != NULL) {
        cout << left << setw(10) << cur->data.maPhieu
             << setw(14) << cur->data.maSinhVien
             << setw(26) << cur->data.tenSinhVien
             << setw(10) << cur->data.maSach
             << setw(32) << cur->data.tenSach
             << setw(14) << cur->data.ngayMuon << "\n";
        cur = cur->next;
    }
    cout << "====================================================================================================\n";
    cout << "Tong so sach dang duoc muon boi sinh vien: " << demSoPhieuMuon(head) << " cuon.\n";
}

void clearListPhieuMuon(NodePhieuMuon*& head) {
    while (head != NULL) {
        NodePhieuMuon* temp = head;
        head = head->next;
        delete temp;
    }
}

// =====================================================================================
// PHAN 6: TIM KIEM NHANH (MANG DONG + QUICK SORT + BINARY SEARCH)
// =====================================================================================

void swapBook(Book& a, Book& b) {
    Book temp = a;
    a = b;
    b = temp;
}

int partition(Book arr[], int low, int high) {
    string pivot = arr[high].maSach;
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (arr[j].maSach < pivot) {
            i++;
            swapBook(arr[i], arr[j]);
        }
    }
    swapBook(arr[i + 1], arr[high]);
    return i + 1;
}

void quickSort(Book arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

int binarySearch(Book arr[], int n, string maCanTim) {
    int left = 0;
    int right = n - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (arr[mid].maSach == maCanTim) return mid;
        if (arr[mid].maSach < maCanTim) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}

bool timKiemSachNhiPhan(NodeSach* head, string maSach, Book& ketQua) {
    int n = demSoLuongDauSach(head);
    if (n == 0) return false;

    // 1. Chuyen Linked List sang Mang dong
    Book* arr = new Book[n];
    NodeSach* cur = head;
    int index = 0;
    while (cur != NULL) {
        arr[index++] = cur->data;
        cur = cur->next;
    }

    // 2. Sap xep mang bang QuickSort
    quickSort(arr, 0, n - 1);

    // 3. Tim kiem bang Binary Search O(log n)
    int viTri = binarySearch(arr, n, maSach);
    bool timThay = false;
    if (viTri != -1) {
        ketQua = arr[viTri];
        timThay = true;
    }

    delete[] arr;
    return timThay;
}

// =====================================================================================
// PHAN 7: DOC VA GHI FILE DU LIEU (FILE I/O)
// =====================================================================================

void luuFileSach(NodeSach* head, const string& tenFile = "data.txt") {
    ofstream outFile(tenFile.c_str());
    if (!outFile.is_open()) {
        cout << "  [LOI] Khong the mo file '" << tenFile << "' de ghi!\n";
        return;
    }

    NodeSach* cur = head;
    while (cur != NULL) {
        outFile << cur->data.maSach << "|"
                << cur->data.tenSach << "|"
                << cur->data.loaiSach << "|"
                << cur->data.tongSoLuong << "|"
                << cur->data.soLuongConLai << "|"
                << cur->data.luotMuon << "\n";
        cur = cur->next;
    }
    outFile.close();
    cout << "  [OK] Da luu thanh cong du lieu sach vao file '" << tenFile << "'!\n";
}

void docFileSach(NodeSach*& head, const string& tenFile = "data.txt") {
    ifstream inFile(tenFile.c_str());
    if (!inFile.is_open()) {
        cout << "  [*] File du lieu '" << tenFile << "' chua ton tai.\n";
        return;
    }

    string line;
    int soLuongDocDuoc = 0;
    while (getline(inFile, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string ma, ten, loai, tongStr, conLaiStr, luotStr;

        if (getline(ss, ma, '|') &&
            getline(ss, ten, '|') &&
            getline(ss, loai, '|') &&
            getline(ss, tongStr, '|') &&
            getline(ss, conLaiStr, '|') &&
            getline(ss, luotStr, '|')) {

            Book b;
            b.maSach = ma;
            b.tenSach = ten;
            b.loaiSach = loai;
            b.tongSoLuong = stoi(tongStr);
            b.soLuongConLai = stoi(conLaiStr);
            b.luotMuon = stoi(luotStr);

            themSachVaoCuoi(head, b);
            soLuongDocDuoc++;
        }
    }
    inFile.close();
    cout << "  [OK] Da nap " << soLuongDocDuoc << " dau sach tu file '" << tenFile << "' vao he thong!\n";
}

void luuFilePhieuMuon(NodePhieuMuon* head, const string& tenFile = "phieu_muon.txt") {
    ofstream outFile(tenFile.c_str());
    if (!outFile.is_open()) return;

    NodePhieuMuon* cur = head;
    while (cur != NULL) {
        outFile << cur->data.maPhieu << "|"
                << cur->data.tenSinhVien << "|"
                << cur->data.maSinhVien << "|"
                << cur->data.maSach << "|"
                << cur->data.tenSach << "|"
                << cur->data.loaiSach << "|"
                << cur->data.ngayMuon << "\n";
        cur = cur->next;
    }
    outFile.close();
    cout << "  [OK] Da luu danh sach phieu muon vao '" << tenFile << "'!\n";
}

void docFilePhieuMuon(NodePhieuMuon*& head, const string& tenFile = "phieu_muon.txt") {
    ifstream inFile(tenFile.c_str());
    if (!inFile.is_open()) return;

    string line;
    int dem = 0;
    while (getline(inFile, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string maP, tenSV, mssv, maS, tenS, loaiS, ngay;

        if (getline(ss, maP, '|') &&
            getline(ss, tenSV, '|') &&
            getline(ss, mssv, '|') &&
            getline(ss, maS, '|') &&
            getline(ss, tenS, '|') &&
            getline(ss, loaiS, '|') &&
            getline(ss, ngay, '|')) {

            PhieuMuon pm = {maP, tenSV, mssv, maS, tenS, loaiS, ngay};
            themPhieuMuon(head, pm);
            dem++;
        }
    }
    inFile.close();
    if (dem > 0) {
        cout << "  [OK] Da nap " << dem << " phieu muon tu file '" << tenFile << "'!\n";
    }
}

// =====================================================================================
// PHAN 8: XUAT BAO CAO RA WEB HTML/CSS (exportToHTML - LIGHT THEME)
// =====================================================================================

void exportToHTML(NodeSach* headSach, NodePhieuMuon* headPM, const string& tenFile = "thong_ke.html") {
    ofstream html(tenFile.c_str());
    if (!html.is_open()) {
        cout << "  [LOI] Khong the tao file HTML '" << tenFile << "'!\n";
        return;
    }

    int tongDauSach = 0, tongSoLuongSach = 0, tongConLai = 0, tongLuotMuon = 0;
    NodeSach* curS = headSach;
    while (curS != NULL) {
        tongDauSach++;
        tongSoLuongSach += curS->data.tongSoLuong;
        tongConLai += curS->data.soLuongConLai;
        tongLuotMuon += curS->data.luotMuon;
        curS = curS->next;
    }
    int soSachDangMuon = tongSoLuongSach - tongConLai;

    html << "<!DOCTYPE html>\n"
         << "<html lang=\"vi\">\n"
         << "<head>\n"
         << "  <meta charset=\"UTF-8\">\n"
         << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
         << "  <title>Cong Thong Ke Thu Vien - Dai Hoc Bach Khoa Ha Noi</title>\n"
         << "  <link rel=\"preconnect\" href=\"https://fonts.googleapis.com\">\n"
         << "  <link rel=\"preconnect\" href=\"https://fonts.gstatic.com\" crossorigin>\n"
         << "  <link href=\"https://fonts.googleapis.com/css2?family=Be+Vietnam+Pro:wght@400;500;600;700&family=JetBrains+Mono:wght@400;500&display=swap\" rel=\"stylesheet\">\n"
         << "  <style>\n"
         << "    :root {\n"
         << "      --bg: #f8fafc; --card: #ffffff; --border: #e2e8f0; --primary: #1e3a8a;\n"
         << "      --text-main: #0f172a; --text-muted: #64748b;\n"
         << "      --green-bg: #ecfdf5; --green-text: #065f46; --green-border: #a7f3d0;\n"
         << "      --red-bg: #fff1f2; --red-text: #9f1239; --red-border: #fecdd3;\n"
         << "    }\n"
         << "    * { margin:0; padding:0; box-sizing:border-box; }\n"
         << "    body {\n"
         << "      font-family: 'Be Vietnam Pro', sans-serif; background: var(--bg); color: var(--text-main);\n"
         << "      min-height: 100vh; line-height: 1.5; font-size: 14px;\n"
         << "    }\n"
         << "    .navbar { background: #fff; border-bottom: 1px solid var(--border); position: sticky; top: 0; z-index: 10; }\n"
         << "    .navbar-inner { max-width: 1140px; margin: 0 auto; padding: 0 24px; height: 60px; display: flex; justify-content: space-between; align-items: center; }\n"
         << "    .brand { display: flex; align-items: center; gap: 10px; }\n"
         << "    .brand-logo { width: 34px; height: 34px; background: var(--primary); color: #fff; border-radius: 7px; display: flex; align-items: center; justify-content: center; font-weight: 700; font-size: 14px; }\n"
         << "    .brand-text h1 { font-size: 15px; font-weight: 700; color: var(--primary); }\n"
         << "    .brand-text p { font-size: 12px; color: var(--text-muted); }\n"
         << "    .main-wrap { max-width: 1140px; margin: 24px auto 60px; padding: 0 24px; }\n"
         << "    .stats-row { display: grid; grid-template-columns: repeat(4, 1fr); gap: 16px; margin-bottom: 24px; }\n"
         << "    .stat-card { background: #fff; border: 1px solid var(--border); border-radius: 8px; padding: 16px 20px; box-shadow: 0 1px 2px rgba(0,0,0,0.04); }\n"
         << "    .stat-card-title { font-size: 12px; font-weight: 600; color: var(--text-muted); text-transform: uppercase; letter-spacing: 0.04em; }\n"
         << "    .stat-card-value { font-size: 26px; font-weight: 700; color: var(--text-main); margin-top: 4px; }\n"
         << "    .table-card { background: #fff; border: 1px solid var(--border); border-radius: 8px; overflow: hidden; box-shadow: 0 1px 2px rgba(0,0,0,0.04); margin-bottom: 24px; }\n"
         << "    .table-header { padding: 16px 20px; border-bottom: 1px solid var(--border); font-size: 15px; font-weight: 700; color: var(--text-main); display: flex; justify-content: space-between; align-items: center; }\n"
         << "    table { width: 100%; border-collapse: collapse; text-align: left; }\n"
         << "    thead th { background: #f8fafc; color: #475569; font-size: 11px; font-weight: 600; text-transform: uppercase; letter-spacing: 0.05em; padding: 12px 20px; border-bottom: 1px solid var(--border); }\n"
         << "    tbody td { padding: 14px 20px; border-bottom: 1px solid #f1f5f9; font-size: 13.5px; vertical-align: middle; }\n"
         << "    tbody tr:last-child td { border-bottom: none; }\n"
         << "    tbody tr:hover { background: #f8fafc; }\n"
         << "    .code-pill { font-family: 'JetBrains Mono', monospace; font-size: 12px; color: #475569; background: #f1f5f9; padding: 3px 6px; border-radius: 4px; border: 1px solid #e2e8f0; }\n"
         << "    .status-pill { display: inline-flex; align-items: center; gap: 6px; padding: 3px 10px; border-radius: 9999px; font-size: 12px; font-weight: 600; }\n"
         << "    .status-pill::before { content: ''; width: 6px; height: 6px; border-radius: 50%; }\n"
         << "    .status-avail { background: var(--green-bg); color: var(--green-text); border: 1px solid var(--green-border); }\n"
         << "    .status-avail::before { background: #10b981; }\n"
         << "    .status-borrow { background: var(--red-bg); color: var(--red-text); border: 1px solid var(--red-border); }\n"
         << "    .status-borrow::before { background: #f43f5e; }\n"
         << "    .footer { text-align: center; margin-top: 36px; color: var(--text-muted); font-size: 12px; }\n"
         << "    @media (max-width: 768px) { .stats-row { grid-template-columns: repeat(2, 1fr); } }\n"
         << "  </style>\n"
         << "</head>\n"
         << "<body>\n"
         << "  <nav class=\"navbar\">\n"
         << "    <div class=\"navbar-inner\">\n"
         << "      <div class=\"brand\">\n"
         << "        <div class=\"brand-logo\">BK</div>\n"
         << "        <div class=\"brand-text\">\n"
         << "          <h1>Thu Vien Dai Hoc Bach Khoa Ha Noi</h1>\n"
         << "          <p>He thong Quan ly Sach &bull; Quan he Sach - Nguoi muon &bull; DSA Project</p>\n"
         << "        </div>\n"
         << "      </div>\n"
         << "    </div>\n"
         << "  </nav>\n"
         << "  <main class=\"main-wrap\">\n"
         << "    <div class=\"stats-row\">\n"
         << "      <div class=\"stat-card\">\n"
         << "        <div class=\"stat-card-title\">Tong so dau sach</div>\n"
         << "        <div class=\"stat-card-value\">" << tongDauSach << "</div>\n"
         << "      </div>\n"
         << "      <div class=\"stat-card\">\n"
         << "        <div class=\"stat-card-title\">Sach con trong kho</div>\n"
         << "        <div class=\"stat-card-value\" style=\"color:#047857;\">" << tongConLai << "/" << tongSoLuongSach << "</div>\n"
         << "      </div>\n"
         << "      <div class=\"stat-card\">\n"
         << "        <div class=\"stat-card-title\">Sach dang duoc muon</div>\n"
         << "        <div class=\"stat-card-value\" style=\"color:#b91c1c;\">" << soSachDangMuon << "</div>\n"
         << "      </div>\n"
         << "      <div class=\"stat-card\">\n"
         << "        <div class=\"stat-card-title\">Tong luot muon</div>\n"
         << "        <div class=\"stat-card-value\" style=\"color:#b45309;\">" << tongLuotMuon << "</div>\n"
         << "      </div>\n"
         << "    </div>\n"
         << "    <!-- BANG 1: DANH MUC SACH -->\n"
         << "    <div class=\"table-card\">\n"
         << "      <div class=\"table-header\">Danh Muc Sach Trong Thu Vien</div>\n"
         << "      <table>\n"
         << "        <thead><tr>\n"
         << "          <th style=\"width:60px;\">STT</th>\n"
         << "          <th style=\"width:110px;\">Ma Sach</th>\n"
         << "          <th>Ten Cuon Sach</th>\n"
         << "          <th style=\"width:140px;\">Loai Sach</th>\n"
         << "          <th style=\"width:120px;\">Tong So</th>\n"
         << "          <th style=\"width:120px;\">Con Lai</th>\n"
         << "          <th style=\"width:110px;\">Luot Muon</th>\n"
         << "          <th style=\"width:130px;\">Tinh Trang</th>\n"
         << "        </tr></thead>\n"
         << "        <tbody>\n";

    curS = headSach;
    int stt = 1;
    while (curS != NULL) {
        html << "          <tr>\n"
             << "            <td>" << stt++ << "</td>\n"
             << "            <td><span class=\"code-pill\">" << curS->data.maSach << "</span></td>\n"
             << "            <td><strong style=\"color:#0f172a;\">" << curS->data.tenSach << "</strong></td>\n"
             << "            <td>" << curS->data.loaiSach << "</td>\n"
             << "            <td>" << curS->data.tongSoLuong << " cuon</td>\n"
             << "            <td><strong>" << curS->data.soLuongConLai << "</strong> cuon</td>\n"
             << "            <td>" << curS->data.luotMuon << " lan</td>\n";
        if (curS->data.soLuongConLai > 0) {
            html << "            <td><span class=\"status-pill status-avail\">Con sach</span></td>\n";
        } else {
            html << "            <td><span class=\"status-pill status-borrow\">Het sach</span></td>\n";
        }
        html << "          </tr>\n";
        curS = curS->next;
    }

    html << "        </tbody>\n"
         << "      </table>\n"
         << "    </div>\n"
         << "    <!-- BANG 2: DANH SACH NGUOI MUON -->\n"
         << "    <div class=\"table-card\">\n"
         << "      <div class=\"table-header\">\n"
         << "        <span>Danh Sach Sinh Vien Dang Muon Sach</span>\n"
         << "        <span style=\"font-size:13px; font-weight:normal; color:#64748b;\">Quan he 1-N (Sach &bull; Nguoi muon)</span>\n"
         << "      </div>\n"
         << "      <table>\n"
         << "        <thead><tr>\n"
         << "          <th style=\"width:60px;\">STT</th>\n"
         << "          <th style=\"width:110px;\">Ma Phieu</th>\n"
         << "          <th style=\"width:130px;\">MSSV</th>\n"
         << "          <th>Ho va Ten Sinh Vien</th>\n"
         << "          <th style=\"width:110px;\">Ma Sach</th>\n"
         << "          <th>Ten Cuon Sach</th>\n"
         << "          <th style=\"width:130px;\">Loai Sach</th>\n"
         << "          <th style=\"width:120px;\">Ngay Muon</th>\n"
         << "        </tr></thead>\n"
         << "        <tbody>\n";

    NodePhieuMuon* curPM = headPM;
    int sttPM = 1;
    while (curPM != NULL) {
        html << "          <tr>\n"
             << "            <td>" << sttPM++ << "</td>\n"
             << "            <td><span class=\"code-pill\">" << curPM->data.maPhieu << "</span></td>\n"
             << "            <td><strong>" << curPM->data.maSinhVien << "</strong></td>\n"
             << "            <td>" << curPM->data.tenSinhVien << "</td>\n"
             << "            <td><span class=\"code-pill\">" << curPM->data.maSach << "</span></td>\n"
             << "            <td>" << curPM->data.tenSach << "</td>\n"
             << "            <td>" << curPM->data.loaiSach << "</td>\n"
             << "            <td>" << curPM->data.ngayMuon << "</td>\n"
             << "          </tr>\n";
        curPM = curPM->next;
    }

    if (sttPM == 1) {
        html << "          <tr><td colspan=\"8\" style=\"text-align: center; color: #64748b; padding: 25px;\">Hien tai chua co sinh vien nao muon sach.</td></tr>\n";
    }

    html << "        </tbody>\n"
         << "      </table>\n"
         << "    </div>\n"
         << "    <div class=\"footer\"><p>Bao cao HTML tu dong &bull; Do an mon Cau truc Du lieu va Giai thuat &bull; C++</p></div>\n"
         << "  </main>\n"
         << "</body>\n"
         << "</html>\n";

    html.close();
    cout << "  [OK] Da xuat bao cao Web thanh cong ra file '" << tenFile << "'!\n";
    cout << "  [*] Ban co the click dup vao file '" << tenFile << "' de xem tren trinh duyet.\n";
}

// =====================================================================================
// PHAN 9: CAC CHUC NANG NGHIEP VU MENU
// =====================================================================================

void chucNangThemSach(NodeSach*& head) {
    cout << "\n--- THEM DAU SACH MOI VAO THU VIEN ---\n";
    Book b;
    cout << "  Nhap ma sach (vi du: CS106): ";
    getline(cin, b.maSach);

    if (b.maSach.empty()) { cout << "  [!] Ma sach khong duoc de trong!\n"; return; }
    if (timSachTheoMa(head, b.maSach) != NULL) {
        cout << "  [!] Ma sach '" << b.maSach << "' da ton tai! Vui long nhap ma khac.\n";
        return;
    }

    cout << "  Nhap ten sach: ";
    getline(cin, b.tenSach);
    if (b.tenSach.empty()) { cout << "  [!] Ten sach khong duoc de trong!\n"; return; }

    cout << "  Nhap loai sach (Giao trinh / Ky thuat / Tham khao / Khoa hoc...): ";
    getline(cin, b.loaiSach);
    if (b.loaiSach.empty()) b.loaiSach = "Tham khao";

    cout << "  Nhap tong so luong sach: ";
    while (!(cin >> b.tongSoLuong) || b.tongSoLuong <= 0) {
        cin.clear(); cin.ignore(1000, '\n');
        cout << "  [!] Vui long nhap so luong nguyen duong: ";
    }
    cin.ignore(1000, '\n');

    b.soLuongConLai = b.tongSoLuong; // Ban dau toan bo sach con trong kho
    b.luotMuon = 0;

    themSachVaoCuoi(head, b);
    cout << "  [OK] Da them dau sach '" << b.tenSach << "' thanh cong voi so luong " << b.tongSoLuong << " cuon!\n";
}

void chucNangXoaSach(NodeSach*& head, Stack& undoStack) {
    cout << "\n--- XOA DAU SACH THEO MA (CO HOAN TAC) ---\n";
    string ma;
    cout << "  Nhap ma sach can xoa: ";
    getline(cin, ma);

    Book sachBiXoa;
    if (xoaSachTheoMa(head, ma, sachBiXoa)) {
        UndoAction action;
        action.loaiHanhDong = ACTION_XOA_SACH;
        action.sach = sachBiXoa;
        push(undoStack, action);

        cout << "  [OK] Da xoa thanh cong dau sach: '" << sachBiXoa.tenSach << "' (Ma: " << sachBiXoa.maSach << ")\n";
        cout << "  [*] Thao tac da duoc luu vao Stack. Ban co the Undo de phuc hoi!\n";
    } else {
        cout << "  [!] Khong tim thay dau sach co ma '" << ma << "' de xoa!\n";
    }
}

// Bien dem tao ma phieu tu dong
int demMaPhieu = 1;

void chucNangMuonSach(NodeSach* headSach, NodePhieuMuon*& headPM, Stack& undoStack, Queue& queueDatCho) {
    cout << "\n--- CHO SINH VIEN MUON SACH ---\n";
    string maSach;
    cout << "  Nhap ma sach can muon: ";
    getline(cin, maSach);

    // Tim sach bang Binary Search O(log n)
    Book sachTim;
    bool timThay = timKiemSachNhiPhan(headSach, maSach, sachTim);
    if (!timThay) {
        cout << "  [!] Khong tim thay ma sach '" << maSach << "' trong thu vien!\n";
        return;
    }

    NodeSach* nodeSach = timSachTheoMa(headSach, maSach);

    // Kiem tra so luong sach con lai
    if (nodeSach->data.soLuongConLai <= 0) {
        cout << "  [!] Cuon sach '" << nodeSach->data.tenSach << "' HIEN DA DUOC MUON HET (Con lai: 0 cuon)!\n";
        cout << "  Ban co muon dang ky xep vao Hang doi dat truoc (Queue) khong? (c/k): ";
        string ans;
        getline(cin, ans);
        if (ans == "c" || ans == "C") {
            PhieuDatCho pdc;
            cout << "  Nhap ho ten sinh vien : ";
            getline(cin, pdc.tenSinhVien);
            cout << "  Nhap ma so sinh vien  : ";
            getline(cin, pdc.maSinhVien);
            pdc.maSachCanMuon = maSach;

            if (!pdc.tenSinhVien.empty() && !pdc.maSinhVien.empty()) {
                enqueue(queueDatCho, pdc);
                cout << "  [OK] Sinh vien " << pdc.tenSinhVien << " (MSSV: " << pdc.maSinhVien
                     << ") da duoc dua vao Hang doi tai vi tri thu " << queueDatCho.count << "!\n";
            }
        }
        return;
    }

    // Nhap thong tin nguoi muon
    PhieuMuon pm;
    cout << "  Nhap ho ten sinh vien muon : ";
    getline(cin, pm.tenSinhVien);
    if (pm.tenSinhVien.empty()) { cout << "  [!] Ho ten khong duoc de trong!\n"; return; }

    cout << "  Nhap ma so sinh vien (MSSV) : ";
    getline(cin, pm.maSinhVien);
    if (pm.maSinhVien.empty()) { cout << "  [!] MSSV khong duoc de trong!\n"; return; }

    cout << "  Nhap ngay muon (vi du: 16/10/2026): ";
    getline(cin, pm.ngayMuon);
    if (pm.ngayMuon.empty()) pm.ngayMuon = "16/10/2026";

    // Tao ma phieu muon
    stringstream ss;
    ss << "PM" << setfill('0') << setw(3) << demMaPhieu++;
    pm.maPhieu = ss.str();
    pm.maSach = nodeSach->data.maSach;
    pm.tenSach = nodeSach->data.tenSach;
    pm.loaiSach = nodeSach->data.loaiSach;

    // Cap nhat so luong sach
    nodeSach->data.soLuongConLai--;
    nodeSach->data.luotMuon++;

    // Luu phieu muon vao danh sach
    themPhieuMuon(headPM, pm);

    // Luu hanh dong vao Stack Undo
    UndoAction act;
    act.loaiHanhDong = ACTION_MUON_SACH;
    act.sach = nodeSach->data;
    act.phieu = pm;
    push(undoStack, act);

    cout << "\n  [OK] Cho muon sach thanh cong!\n";
    cout << "  + Ma phieu muon: " << pm.maPhieu << "\n";
    cout << "  + Sinh vien    : " << pm.tenSinhVien << " (MSSV: " << pm.maSinhVien << ")\n";
    cout << "  + Sach muon    : " << pm.tenSach << " [" << pm.loaiSach << "]\n";
    cout << "  + So luong con : " << nodeSach->data.soLuongConLai << "/" << nodeSach->data.tongSoLuong << " cuon\n";
    cout << "  [*] Thao tac da duoc luu vao Stack. Ban co the Undo neu can.\n";
}

void chucNangTraSach(NodeSach* headSach, NodePhieuMuon*& headPM, Queue& queueDatCho) {
    cout << "\n--- NHAN TRA SACH TU SINH VIEN ---\n";
    string mssv, maSach;
    cout << "  Nhap ma so sinh vien (MSSV) tra sach: ";
    getline(cin, mssv);
    cout << "  Nhap ma sach can tra: ";
    getline(cin, maSach);

    PhieuMuon phieuXoa;
    bool timThay = xoaPhieuMuon(headPM, mssv, maSach, phieuXoa);

    if (!timThay) {
        cout << "  [!] Khong tim thay phieu muon cua sinh vien MSSV '" << mssv
             << "' voi ma sach '" << maSach << "'!\n";
        return;
    }

    // Tang lai so luong sach con trong kho
    NodeSach* nodeSach = timSachTheoMa(headSach, maSach);
    if (nodeSach != NULL) {
        if (nodeSach->data.soLuongConLai < nodeSach->data.tongSoLuong) {
            nodeSach->data.soLuongConLai++;
        }
    }

    cout << "  [OK] Da nhan tra sach: '" << phieuXoa.tenSach << "' tu sinh vien "
         << phieuXoa.tenSinhVien << " (MSSV: " << mssv << ")!\n";
    if (nodeSach != NULL) {
        cout << "  So luong sach hien tai trong kho: " << nodeSach->data.soLuongConLai
             << "/" << nodeSach->data.tongSoLuong << " cuon.\n";
    }

    // Kiem tra hang doi dat truoc
    if (!isQueueEmpty(queueDatCho) && queueDatCho.front->data.maSachCanMuon == maSach) {
        PhieuDatCho pdc;
        dequeue(queueDatCho, pdc);
        cout << "  ---------------------------------------------------------\n";
        cout << "  [THONG BAO UU TIEN] Sinh vien: " << pdc.tenSinhVien
             << " (MSSV: " << pdc.maSinhVien << ") dang xep dau hang doi cho sach nay!\n";
        cout << "  Vui long uu tien lam thu tuc muon cho sinh vien tren.\n";
        cout << "  ---------------------------------------------------------\n";
    }
}

void chucNangTimKiem(NodeSach* head) {
    cout << "\n--- TIM KIEM SACH (MANG DONG + QUICK SORT + BINARY SEARCH) ---\n";
    string ma;
    cout << "  Nhap ma sach can tra cuu: ";
    getline(cin, ma);

    Book ketQua;
    bool timThay = timKiemSachNhiPhan(head, ma, ketQua);

    if (timThay) {
        cout << "\n  [KET QUA TIM KIEM NHI PHAN O(log n)]:\n";
        cout << "  + Ma sach   : " << ketQua.maSach << "\n";
        cout << "  + Ten sach  : " << ketQua.tenSach << "\n";
        cout << "  + Loai sach : " << ketQua.loaiSach << "\n";
        cout << "  + Tong so   : " << ketQua.tongSoLuong << " cuon\n";
        cout << "  + Con lai   : " << ketQua.soLuongConLai << " cuon\n";
        cout << "  + Luot muon : " << ketQua.luotMuon << " lan\n";
        cout << "  + Tinh trang: " << ((ketQua.soLuongConLai > 0) ? "Con sach de muon" : "Da het sach") << "\n";
    } else {
        cout << "  [!] Khong tim thay bat ky dau sach nao co ma '" << ma << "'!\n";
    }
}

void chucNangDatCho(NodeSach* head, Queue& queueDatCho) {
    cout << "\n--- DANG KY DAT TRUOC SACH (VAO HANG DOI QUEUE) ---\n";
    string ma;
    cout << "  Nhap ma sach ban muon dat truoc: ";
    getline(cin, ma);

    NodeSach* node = timSachTheoMa(head, ma);
    if (node == NULL) { cout << "  [!] Ma sach khong ton tai!\n"; return; }

    if (node->data.soLuongConLai > 0) {
        cout << "  [*] Cuon sach '" << node->data.tenSach << "' van CON LAI "
             << node->data.soLuongConLai << " cuon trong kho! Ban co the muon truc tiep.\n";
        return;
    }

    PhieuDatCho pdc;
    cout << "  Nhap ho ten sinh vien : ";
    getline(cin, pdc.tenSinhVien);
    cout << "  Nhap ma so sinh vien  : ";
    getline(cin, pdc.maSinhVien);
    pdc.maSachCanMuon = ma;

    if (!pdc.tenSinhVien.empty() && !pdc.maSinhVien.empty()) {
        enqueue(queueDatCho, pdc);
        cout << "  [OK] Sinh vien " << pdc.tenSinhVien << " da vao hang doi dat truoc sach '"
             << node->data.tenSach << "' tai vi tri thu " << queueDatCho.count << "!\n";
    }
}

void chucNangPhucVuHangDoi(NodeSach* headSach, NodePhieuMuon*& headPM, Queue& queueDatCho, Stack& undoStack) {
    cout << "\n--- PHUC VU SINH VIEN DAU TIEN TRONG HANG DOI (DEQUEUE) ---\n";
    if (isQueueEmpty(queueDatCho)) {
        cout << "  [!] Hang doi hien dang trong!\n";
        return;
    }

    PhieuDatCho pdc;
    dequeue(queueDatCho, pdc);
    cout << "  Dang xu ly dat cho: " << pdc.tenSinhVien << " (MSSV: " << pdc.maSinhVien << ")\n";

    NodeSach* node = timSachTheoMa(headSach, pdc.maSachCanMuon);
    if (node == NULL) {
        cout << "  [!] Dau sach nay khong con ton tai!\n";
        return;
    }

    if (node->data.soLuongConLai <= 0) {
        cout << "  [!] Hien tai sach van chua co ai tra, vui long cho!\n";
        return;
    }

    // Lap phieu muon cho sinh vien
    PhieuMuon pm;
    stringstream ss;
    ss << "PM" << setfill('0') << setw(3) << demMaPhieu++;
    pm.maPhieu = ss.str();
    pm.tenSinhVien = pdc.tenSinhVien;
    pm.maSinhVien = pdc.maSinhVien;
    pm.maSach = node->data.maSach;
    pm.tenSach = node->data.tenSach;
    pm.loaiSach = node->data.loaiSach;
    pm.ngayMuon = "16/10/2026";

    node->data.soLuongConLai--;
    node->data.luotMuon++;
    themPhieuMuon(headPM, pm);

    UndoAction act;
    act.loaiHanhDong = ACTION_MUON_SACH;
    act.sach = node->data;
    act.phieu = pm;
    push(undoStack, act);

    cout << "  [OK] Da giai quyet cho sinh vien " << pdc.tenSinhVien
         << " muon sach '" << node->data.tenSach << "' thanh cong!\n";
}

void chucNangUndo(NodeSach* headSach, NodePhieuMuon*& headPM, Stack& undoStack) {
    cout << "\n--- HOAN TAC THAO TAC VUA THUC HIEN (UNDO) ---\n";
    if (isStackEmpty(undoStack)) {
        cout << "  [!] Stack rong! Khong co thao tac nao de hoan tac.\n";
        return;
    }

    UndoAction act;
    pop(undoStack, act);

    if (act.loaiHanhDong == ACTION_MUON_SACH) {
        // Hoan tac muon: Huy phieu muon va tang lai so luong con lai
        PhieuMuon pXoa;
        xoaPhieuMuon(headPM, act.phieu.maSinhVien, act.phieu.maSach, pXoa);

        NodeSach* node = timSachTheoMa(headSach, act.sach.maSach);
        if (node != NULL) {
            node->data.soLuongConLai++;
            if (node->data.luotMuon > 0) node->data.luotMuon--;
            cout << "  [OK] Da hoan tac thao tac MUON SACH!\n";
            cout << "  Huy phieu muon " << act.phieu.maPhieu << " cua SV " << act.phieu.tenSinhVien << ".\n";
            cout << "  So luong con lai cua sach '" << node->data.tenSach << "' tang len: "
                 << node->data.soLuongConLai << " cuon.\n";
        }
    }
    else if (act.loaiHanhDong == ACTION_XOA_SACH) {
        themSachVaoCuoi(headSach, act.sach);
        cout << "  [OK] Da hoan tac thao tac XOA SACH!\n";
        cout << "  Dau sach '" << act.sach.tenSach << "' (Ma: " << act.sach.maSach << ") da duoc phuc hoi vao kho.\n";
    }
}

// =====================================================================================
// PHAN 10: GIAO DIEN CONSOLE MENU CHINH
// =====================================================================================

void hienThiMenu(int soLuongUndo, int soLuongQueue, int soPhieuMuon) {
    cout << "\n====================================================================\n";
    cout << "       HE THONG QUAN LY THU VIEN - DAI HOC BACH KHOA HA NOI        \n";
    cout << "====================================================================\n";
    cout << "  [1]  Xem danh muc toan bo sach trong thu vien\n";
    cout << "  [2]  Xem danh sach sinh vien dang muon sach (Phieu muon)\n";
    cout << "  [3]  Them dau sach moi vao kho\n";
    cout << "  [4]  Xoa dau sach theo ma (Co ho tro Undo)\n";
    cout << "  [5]  Cho sinh vien muon sach (Nhap ten, MSSV, loai sach)\n";
    cout << "  [6]  Nhan tra sach tu sinh vien\n";
    cout << "  [7]  Tim kiem sach theo ma (Mang dong + QuickSort + Binary Search)\n";
    cout << "  [8]  Dang ky dat cho khi sach het (Vao hang doi Queue)\n";
    cout << "  [9]  Xem hang doi dat cho (Queue FIFO)\n";
    cout << "  [10] Phuc vu sinh vien tiep theo trong hang doi (Dequeue)\n";
    cout << "  [11] Hoan tac thao tac vua thuc hien (Undo bang Stack)\n";
    cout << "  [12] Xuat bao cao thong ke ra file Web (thong_ke.html)\n";
    cout << "  [13] Luu toan bo du lieu ra file (data.txt & phieu_muon.txt)\n";
    cout << "  [0]  Thoat chuong trinh\n";
    cout << "--------------------------------------------------------------------\n";
    cout << "  [*] Trang thai: Stack Undo (" << soLuongUndo << " thao tac) | "
         << "Hang doi (" << soLuongQueue << " nguoi) | "
         << "Dang muon (" << soPhieuMuon << " cuon)\n";
    cout << "====================================================================\n";
    cout << "  >> Nhap lua chon cua ban [0-13]: ";
}

// =====================================================================================
// PHAN 11: HAM MAIN - DIEM KHOI DAU CHUONG TRINH
// =====================================================================================

int main() {
    NodeSach* headSach = NULL;
    NodePhieuMuon* headPM = NULL;
    Stack undoStack;
    Queue queueDatCho;

    initStack(undoStack);
    initQueue(queueDatCho);

    cout << "====================================================================\n";
    cout << "  KHOI DONG HE THONG QUAN LY THU VIEN (DSA MINI PROJECT)\n";
    cout << "====================================================================\n";

    // Doc du lieu tu file khi khoi dong
    docFileSach(headSach, "data.txt");
    docFilePhieuMuon(headPM, "phieu_muon.txt");

    // Neu chua co sach mau nao, tao san du lieu phong phu
    if (headSach == NULL) {
        cout << "  [*] Tu dong tao mot so dau sach mau kem so luong kho...\n";
        themSachVaoCuoi(headSach, {"CS101", "Cau truc du lieu va Giai thuat", "Giao trinh", 10, 8, 15});
        themSachVaoCuoi(headSach, {"CS102", "Lap trinh huong doi tuong C++", "Giao trinh", 5, 2, 28});
        themSachVaoCuoi(headSach, {"CS103", "Kien truc may tinh va He dieu hanh", "Ky thuat", 8, 7, 9});
        themSachVaoCuoi(headSach, {"CS104", "Co so du lieu quan he SQL", "Ky thuat", 6, 5, 12});
        themSachVaoCuoi(headSach, {"CS105", "Tri tue nhan tao can ban", "Khoa hoc", 4, 1, 34});
        luuFileSach(headSach, "data.txt");
    }

    // Neu chua co phieu muon nao, tao vai phieu mau de test quan he nguoi muon
    if (headPM == NULL) {
        themPhieuMuon(headPM, {"PM001", "Nguyen Van An", "20210001", "CS101", "Cau truc du lieu va Giai thuat", "Giao trinh", "15/10/2026"});
        themPhieuMuon(headPM, {"PM002", "Tran Thi Binh", "20210002", "CS101", "Cau truc du lieu va Giai thuat", "Giao trinh", "15/10/2026"});
        themPhieuMuon(headPM, {"PM003", "Le Van Cuong", "20210003", "CS102", "Lap trinh huong doi tuong C++", "Giao trinh", "16/10/2026"});
        demMaPhieu = 4;
        luuFilePhieuMuon(headPM, "phieu_muon.txt");
    }

    int luaChon = -1;
    bool tiepTuc = true;

    while (tiepTuc) {
        hienThiMenu(undoStack.count, queueDatCho.count, demSoPhieuMuon(headPM));

        if (!(cin >> luaChon)) {
            if (cin.eof()) break;
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "  [!] Lua chon khong hop le! Vui long nhap so tu 0 den 13.\n";
            continue;
        }
        cin.ignore(1000, '\n');

        switch (luaChon) {
            case 1:
                inDanhSachSachConsole(headSach);
                break;
            case 2:
                inDanhSachPhieuMuonConsole(headPM);
                break;
            case 3:
                chucNangThemSach(headSach);
                break;
            case 4:
                chucNangXoaSach(headSach, undoStack);
                break;
            case 5:
                chucNangMuonSach(headSach, headPM, undoStack, queueDatCho);
                break;
            case 6:
                chucNangTraSach(headSach, headPM, queueDatCho);
                break;
            case 7:
                chucNangTimKiem(headSach);
                break;
            case 8:
                chucNangDatCho(headSach, queueDatCho);
                break;
            case 9:
                inHangDoiConsole(queueDatCho);
                break;
            case 10:
                chucNangPhucVuHangDoi(headSach, headPM, queueDatCho, undoStack);
                break;
            case 11:
                chucNangUndo(headSach, headPM, undoStack);
                break;
            case 12:
                exportToHTML(headSach, headPM, "thong_ke.html");
                break;
            case 13:
                luuFileSach(headSach, "data.txt");
                luuFilePhieuMuon(headPM, "phieu_muon.txt");
                break;
            case 0:
                cout << "\n  Dang luu du lieu vao file truoc khi thoat...\n";
                luuFileSach(headSach, "data.txt");
                luuFilePhieuMuon(headPM, "phieu_muon.txt");
                cout << "  Cam on ban da su dung chuong trinh! Chuc ban bao ve do an dat diem cao!\n\n";
                tiepTuc = false;
                break;
            default:
                cout << "  [!] Lua chon khong ton tai! Vui long nhap tu 0 den 13.\n";
                break;
        }

        if (tiepTuc) {
            cout << "\n  >> Nhan Enter de tiep tuc...";
            string enterTam;
            getline(cin, enterTam);
        }
    }

    // Giai phong bo nho dong truoc khi thoat
    clearListSach(headSach);
    clearListPhieuMuon(headPM);
    clearStack(undoStack);
    clearQueue(queueDatCho);

    return 0;
}
