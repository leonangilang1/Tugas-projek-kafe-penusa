#include <windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <ctime> // Untuk time_t dan strftime

// Definisi ID tombol
#define ID_LOGIN_BUTTON 1
#define ID_EXIT_BUTTON 2
#define ID_TAMBAH_BUTTON 3
#define ID_VIEW_BUTTON 4
#define ID_SIMPAN_BUTTON 5  // ID untuk tombol Simpan
#define ID_KEMBALI_BUTTON 6 // ID untuk tombol Kembali
#define ID_PREVIEW_BUTTON 8  // ID untuk tombol Print Preview

// Username dan Password untuk login
std::string username = "admin";
std::string password = "admin";

// Deklarasi global untuk menyimpan data penjualan
std::vector<std::string> penjualanData;  // Menyimpan data penjualan

// Handle untuk elemen-elemen
HWND hwndUsername, hwndPassword, hwndLoginButton;
HWND hwndMainWindow;  // Global handle untuk jendela utama
HWND hwndTambahPenjualan = nullptr;  // Global handle untuk jendela "Tambah Penjualan"
HMENU hMenu;

// Variabel untuk form input penjualan
HWND hwndNoPesanan, hwndTanggalTransaksi, hwndNoMeja, hwndJenisPesanan;
HWND hwndNamaJenisPesanan, hwndHargaSatuan, hwndJumlahBeli, hwndHarga;

// Fungsi untuk membuat nomor pesanan unik
std::string GenerateUniqueOrderID() {
    // Mengambil waktu saat ini
    time_t now = time(0);
    tm* ltm = localtime(&now);

    // Format: YYYYMMDDHHMMSS
    char buffer[15];
    strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", ltm);

    return std::string(buffer);
}

void PrintPreview(HWND hwnd) {
    PRINTDLG pd = { 0 };
    pd.lStructSize = sizeof(PRINTDLG);
    pd.hwndOwner = hwnd;
    pd.Flags = PD_RETURNDC | PD_PRINTTOFILE | PD_SHOWHELP;
    pd.nCopies = 1;

    if (PrintDlg(&pd)) {
        HDC hdcPrinter = pd.hDC;
        if (hdcPrinter) {
            DOCINFO di = { sizeof(DOCINFO), "Data Penjualan" };
            if (StartDoc(hdcPrinter, &di) > 0) {
                if (StartPage(hdcPrinter) > 0) {
                    int y = 100; // Mulai dari posisi vertikal awal
                    int x = 100; // Mulai dari posisi horizontal awal

                    // Mengatur font dengan ukuran 12 point
                    HFONT hFont = CreateFont(
                        -MulDiv(12, GetDeviceCaps(hdcPrinter, LOGPIXELSY), 72), // 12 point
                        0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
                    
                    if (hFont == NULL) {
                        MessageBox(hwnd, "Gagal membuat font.", "Error", MB_OK | MB_ICONERROR);
                        return;
                    }

                    // Memilih font ke HDC
                    SelectObject(hdcPrinter, hFont);

                    // Mendapatkan tinggi font
                    TEXTMETRIC tm;
                    GetTextMetrics(hdcPrinter, &tm);
                    int fontHeight = tm.tmHeight; // Tinggi font

                    // Mengatur jarak antar baris
                    int lineSpacing = fontHeight + 5; // Tambahkan sedikit ruang ekstra

                    // Header Kafe
                    TextOut(hdcPrinter, x, y, "Kafe Pelita Nusantara", 21);
                    y += lineSpacing; // Menggunakan line spacing
                    TextOut(hdcPrinter, x, y, "Jl. Medan Lubuk Pakam", 21);
                    y += lineSpacing;
                    TextOut(hdcPrinter, x, y, "______________________________", 30);
                    y += lineSpacing;

                    // Tanggal dan No Meja
                    SYSTEMTIME st;
                    GetLocalTime(&st);  // Mengambil tanggal saat ini
                    std::stringstream ss;
                    ss << "Tanggal: " << std::setw(2) << std::setfill('0') << st.wDay << "/"
                       << std::setw(2) << std::setfill('0') << st.wMonth << "/"
                       << st.wYear;

                    TextOut(hdcPrinter, x, y, ss.str().c_str(), ss.str().length());
                    y += lineSpacing;

                    ss.str("");  // Clear stringstream
                    ss << "No Meja: " << 1;  // Misalnya No Meja 1, sesuaikan dengan input yang ada
                    TextOut(hdcPrinter, x, y, ss.str().c_str(), ss.str().length());
                    y += lineSpacing;

                    // Tabel Penjualan
                    y += 35;
                    // Menyusun tabel header
                    TextOut(hdcPrinter, x, y, "Item", 8);
                    TextOut(hdcPrinter, x + 100, y, " ", 10);
                    TextOut(hdcPrinter, x + 200, y, " ", 12);
                    TextOut(hdcPrinter, x + 300, y, " ", 11);
                    y += lineSpacing;

                    // Garis Horizontal setelah header
                    MoveToEx(hdcPrinter, x, y, NULL);
                    LineTo(hdcPrinter, x + 500, y); // Membuat garis horizontal
                    y += 5;

                    // Menampilkan Data Penjualan dalam tabel
                    int totalKeseluruhan = 0;
                    for (const auto& data : penjualanData) {
                        size_t pos = data.find("Nama: ");
                        if (pos != std::string::npos) {
                            // Mengambil nama pesanan
                            std::string namaPesanan = data.substr(pos + 6, data.find("\n", pos) - (pos + 6));

                            // Mengambil harga satuan
                            size_t hargaPos = data.find("Harga Satuan: ");
                            std::string hargaSatuanStr = data.substr(hargaPos + 14, data.find("\n", hargaPos) - (hargaPos + 14));

                            // Mengambil jumlah
                            size_t jumlahPos = data.find("Jumlah: ");
                            std::string jumlahStr = data.substr(jumlahPos + 8, data.find("\n", jumlahPos) - (jumlahPos + 8));

                            // Mengonversi string ke integer
                            int hargaSatuan = std::stoi(hargaSatuanStr);
                            int jumlah = std::stoi(jumlahStr);
                            int totalHarga = hargaSatuan * jumlah;

                            // Menambahkan total harga ke total keseluruhan
                            totalKeseluruhan += totalHarga;

                            // Membuat string untuk menampilkan format "Nama Item HargaSatuan x Jumlah = TotalHarga"
                            std::stringstream outputStream;
                            //outputStream << namaPesanan << " " << jumlah << " " << hargaSatuan << " " << totalHarga;
                            outputStream << namaPesanan << " ";

                            // Menampilkan baris item penjualan
                            TextOut(hdcPrinter, x, y, outputStream.str().c_str(), outputStream.str().length());
                            y += lineSpacing; // Baris berikutnya dengan line spacing
                        }
                    }

                    // Menampilkan total keseluruhan
                    std::stringstream totalStream;
                    totalStream << "Total: Rp." << totalKeseluruhan;
                    TextOut(hdcPrinter, x, y, totalStream.str().c_str(), totalStream.str().length());
                    y += lineSpacing;

                    // Link Kritik dan Saran
                    TextOut(hdcPrinter, x, y, "Link Kritik dan Saran:", 22);
                    y += lineSpacing;
                    TextOut(hdcPrinter, x, y, "bit.ly/kafePenusa", 17);

                    // Menghapus font setelah selesai
                    DeleteObject(hFont);

                    EndPage(hdcPrinter);
                }
                EndDoc(hdcPrinter);
            }
            DeleteDC(hdcPrinter);
        }
    } else {
        MessageBox(hwnd, "Dialog cetak dibatalkan atau gagal.", "Info", MB_OK | MB_ICONINFORMATION);
    }
}

// Fungsi untuk membuat jendela login
void CreateLoginWindow(HINSTANCE hInstance, int nCmdShow);
void CreateMainWindow(HINSTANCE hInstance, HWND hwndParent);
void CreateTambahPenjualanWindow(HINSTANCE hInstance, HWND hwndParent);
void AddPenjualan(HWND hwnd);
void ViewPenjualan(HWND hwnd);

// Fungsi untuk mendapatkan tanggal saat ini dalam format "DD/MM/YYYY"
std::string GetCurrentDate() {
    SYSTEMTIME st;
    GetLocalTime(&st);  // Mengambil tanggal dan waktu saat ini

    // Format: DD/MM/YYYY
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << st.wDay << "/"
       << std::setw(2) << std::setfill('0') << st.wMonth << "/"
       << st.wYear;
    
    return ss.str();
}

// Prosedur untuk menangani event pada jendela login dan utama
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Fungsi untuk memulai aplikasi
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Membuat jendela login dan menjalankan message loop
    CreateLoginWindow(hInstance, nCmdShow);
    
    // Masukkan message loop utama aplikasi
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return msg.wParam;
}

void CreateLoginWindow(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "LoginWindow";
    
    // Registrasi kelas jendela
    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Failed to register window class", "Error", MB_ICONERROR);
        return;
    }

    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, "Login Kasir Kafe Penusa", WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, 300, 200, nullptr, nullptr, hInstance, nullptr);

    // Membuat elemen form login
    CreateWindow("static", "Username:", WS_VISIBLE | WS_CHILD, 20, 30, 80, 20, hwnd, nullptr, hInstance, nullptr);
    hwndUsername = CreateWindow("edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 100, 30, 150, 20, hwnd, nullptr, hInstance, nullptr);
    
    CreateWindow("static", "Password:", WS_VISIBLE | WS_CHILD, 20, 70, 80, 20, hwnd, nullptr, hInstance, nullptr);
    hwndPassword = CreateWindow("edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD, 100, 70, 150, 20, hwnd, nullptr, hInstance, nullptr);

    hwndLoginButton = CreateWindow("button", "Login", WS_VISIBLE | WS_CHILD, 100, 110, 80, 30, hwnd, (HMENU)ID_LOGIN_BUTTON, hInstance, nullptr);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
}

void CreateMainWindow(HINSTANCE hInstance, HWND hwndParent) {
    // Jika MainWindow sudah ada, tidak perlu membuat lagi
    if (hwndMainWindow != nullptr) {
        return;
    }

    // Membuat jendela utama kasir
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MainWindow";
    RegisterClass(&wc);

    hwndMainWindow = CreateWindowEx(0, wc.lpszClassName, "Kasir Kafe Penusa", WS_OVERLAPPEDWINDOW,
                                    CW_USEDEFAULT, CW_USEDEFAULT, 850, 700, nullptr, nullptr, hInstance, hwndParent);

    // Membuat menu utama
    hMenu = CreateMenu();
    
    // Sub-menu File
    AppendMenu(hMenu, MF_STRING, ID_EXIT_BUTTON, "Exit");

    // Sub-menu Record
    HMENU hRecordMenu = CreateMenu();
    AppendMenu(hRecordMenu, MF_STRING, ID_TAMBAH_BUTTON, "Tambah Data Penjualan");
    AppendMenu(hRecordMenu, MF_STRING, ID_VIEW_BUTTON, "Data Penjualan");
    AppendMenu(hRecordMenu, MF_STRING, ID_PREVIEW_BUTTON, "Print Data Penjualan");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hRecordMenu, "Record");

    SetMenu(hwndMainWindow, hMenu);

    // Menampilkan jendela utama
    ShowWindow(hwndMainWindow, SW_SHOWNORMAL);
    UpdateWindow(hwndMainWindow);
}

void CreateTambahPenjualanWindow(HINSTANCE hInstance, HWND hwndParent) {  
    if (hwndTambahPenjualan != nullptr) {  
        // Jika jendela sudah ada, hanya tampilkan  
        ShowWindow(hwndTambahPenjualan, SW_SHOW);  
        SetFocus(hwndTambahPenjualan);  // Fokuskan jendela  
        return;  
    }  
  
    // Jika form belum ada, kita hanya akan menambahkan elemen pada jendela utama (hwndParent)  
    // Membuat elemen form input di dalam hwndParent  
  
    CreateWindow("static", "No. Pesanan:", WS_VISIBLE | WS_CHILD, 20, 30, 100, 20, hwndParent, nullptr, hInstance, nullptr);  
    hwndNoPesanan = CreateWindow("edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY, 150, 30, 200, 20, hwndParent, nullptr, hInstance, nullptr);  
  
    // Set nomor pesanan otomatis  
    std::string uniqueOrderID = GenerateUniqueOrderID();  
    SetWindowText(hwndNoPesanan, uniqueOrderID.c_str());  
  
    CreateWindow("static", "Tanggal Transaksi:", WS_VISIBLE | WS_CHILD, 20, 70, 100, 20, hwndParent, nullptr, hInstance, nullptr);  
    hwndTanggalTransaksi = CreateWindow("edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, 70, 200, 20, hwndParent, nullptr, hInstance, nullptr);  
  
    // Set tanggal otomatis di input Tanggal Transaksi  
    std::string currentDate = GetCurrentDate();  
    SetWindowText(hwndTanggalTransaksi, currentDate.c_str());  
  
    // Membuat elemen lainnya seperti input dan combobox tetap sama seperti sebelumnya...  
  
    CreateWindow("static", "No. Meja:", WS_VISIBLE | WS_CHILD, 20, 110, 100, 20, hwndParent, nullptr, hInstance, nullptr);  
    hwndNoMeja = CreateWindow("combobox", "", WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST,   
                              150, 110, 200, 200, hwndParent, nullptr, hInstance, nullptr);  
    for (int i = 1; i <= 15; i++) {  
        std::string meja = std::to_string(i);  
        SendMessage(hwndNoMeja, CB_ADDSTRING, 0, (LPARAM)meja.c_str());  
    }  
    SendMessage(hwndNoMeja, CB_SETCURSEL, 0, 0);  
  
    CreateWindow("static", "Jenis Pesanan:", WS_VISIBLE | WS_CHILD, 20, 150, 100, 20, hwndParent, nullptr, hInstance, nullptr);  
    hwndJenisPesanan = CreateWindow("combobox", "", WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST,   
                                    150, 150, 200, 200, hwndParent, nullptr, hInstance, nullptr);  
    SendMessage(hwndJenisPesanan, CB_ADDSTRING, 0, (LPARAM)"Makanan");  
    SendMessage(hwndJenisPesanan, CB_ADDSTRING, 0, (LPARAM)"Minuman");  
    SendMessage(hwndJenisPesanan, CB_SETCURSEL, 0, 0);  
      
    CreateWindow("STATIC", "Nama Jenis Pesanan:", WS_VISIBLE | WS_CHILD,   
                 20, 190, 120, 20, hwndParent, nullptr, hInstance, nullptr);  
  
    hwndNamaJenisPesanan = CreateWindow("COMBOBOX", "", WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST,   
                                        150, 190, 200, 300, hwndParent, nullptr, hInstance, nullptr);  
  
    // Menambahkan pilihan-pilihan menu ke ComboBox  
    SendMessage(hwndNamaJenisPesanan, CB_ADDSTRING, 0, (LPARAM)"Nasi Goreng");  
    SendMessage(hwndNamaJenisPesanan, CB_ADDSTRING, 0, (LPARAM)"Mie Goreng");  
    SendMessage(hwndNamaJenisPesanan, CB_ADDSTRING, 0, (LPARAM)"Kuetiaw Goreng");  
    SendMessage(hwndNamaJenisPesanan, CB_ADDSTRING, 0, (LPARAM)"Indomie Rebus");  
    SendMessage(hwndNamaJenisPesanan, CB_ADDSTRING, 0, (LPARAM)"Ayam Penyet");  
    SendMessage(hwndNamaJenisPesanan, CB_ADDSTRING, 0, (LPARAM)"Ayam Cabe Ijo");  
    SendMessage(hwndNamaJenisPesanan, CB_ADDSTRING, 0, (LPARAM)"Jus Kueni");  
    SendMessage(hwndNamaJenisPesanan, CB_ADDSTRING, 0, (LPARAM)"Jus Jeruk");  
    SendMessage(hwndNamaJenisPesanan, CB_ADDSTRING, 0, (LPARAM)"Jus Alpukat");  
    SendMessage(hwndNamaJenisPesanan, CB_ADDSTRING, 0, (LPARAM)"Es Teh Manis");  
    SendMessage(hwndNamaJenisPesanan, CB_ADDSTRING, 0, (LPARAM)"Teh Tarik");  
    SendMessage(hwndNamaJenisPesanan, CB_ADDSTRING, 0, (LPARAM)"Kopi Susu");  
  
    CreateWindow("static", "Harga Satuan:", WS_VISIBLE | WS_CHILD, 20, 230, 100, 20, hwndParent, nullptr, hInstance, nullptr);  
    hwndHargaSatuan = CreateWindow("edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, 230, 200, 20, hwndParent, nullptr, hInstance, nullptr);  
  
    CreateWindow("static", "Jumlah Beli:", WS_VISIBLE | WS_CHILD, 20, 270, 100, 20, hwndParent, nullptr, hInstance, nullptr);  
    hwndJumlahBeli = CreateWindow("edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, 270, 200, 20, hwndParent, nullptr, hInstance, nullptr);  
  
    CreateWindow("button", "Simpan", WS_VISIBLE | WS_CHILD, 150, 340, 80, 30, hwndParent, (HMENU)ID_SIMPAN_BUTTON, hInstance, nullptr);  
  
    ShowWindow(hwndParent, SW_SHOWNORMAL);  
    UpdateWindow(hwndParent);  
}  


void AddPenjualan(HWND hwnd) {  
    char noPesanan[256], tanggal[256], meja[256], jenisPesanan[256], namaPesanan[256];  
    char harga[256], jumlah[256];  
    int hargaSatuan, jumlahBeli;  
  
    GetWindowText(hwndNoPesanan, noPesanan, sizeof(noPesanan));  
    GetWindowText(hwndTanggalTransaksi, tanggal, sizeof(tanggal));  
    GetWindowText(hwndNoMeja, meja, sizeof(meja));  
    GetWindowText(hwndJenisPesanan, jenisPesanan, sizeof(jenisPesanan));  
    GetWindowText(hwndNamaJenisPesanan, namaPesanan, sizeof(namaPesanan));  
    GetWindowText(hwndHargaSatuan, harga, sizeof(harga));  
    GetWindowText(hwndJumlahBeli, jumlah, sizeof(jumlah));  
  
    hargaSatuan = std::stoi(harga);  
    jumlahBeli = std::stoi(jumlah);  
  
    std::stringstream ss;  
    ss << "No Pesanan: " << noPesanan << ", Tanggal: " << tanggal  
       << ", Meja: " << meja << ", Jenis: " << jenisPesanan  
       << ", Nama: " << namaPesanan << ", Harga Satuan: " << hargaSatuan  
       << ", Jumlah: " << jumlahBeli << ", Total Harga: " << (hargaSatuan * jumlahBeli) << "\n";  
      
    penjualanData.push_back(ss.str());  
  
    MessageBox(hwnd, "Data Penjualan berhasil disimpan", "Info", MB_OK);  
  
    // Menghasilkan nomor pesanan baru  
    std::string uniqueOrderID = GenerateUniqueOrderID();  
    SetWindowText(hwndNoPesanan, uniqueOrderID.c_str());  
}  


void ViewPenjualan(HWND hwnd) {
    std::stringstream ss;
    int totalKeseluruhan = 0;
    for (auto& data : penjualanData) {
        ss << data << "\n";

        size_t pos = data.find("Total Harga: ");
        if (pos != std::string::npos) {
            std::string totalString = data.substr(pos + 13);
            totalKeseluruhan += std::stoi(totalString);
        }
    }

    ss << "\nTotal Keseluruhan: " << totalKeseluruhan;

    MessageBox(hwnd, ss.str().c_str(), "Data Penjualan", MB_OK);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_LOGIN_BUTTON:
            CreateMainWindow(GetModuleHandle(NULL), hwnd);
            break;
        case ID_EXIT_BUTTON:
            PostQuitMessage(0);
            break;
        case ID_TAMBAH_BUTTON:
            CreateTambahPenjualanWindow(GetModuleHandle(NULL), hwnd);
            break;
        case ID_VIEW_BUTTON:
            ViewPenjualan(hwnd);
            break;
        case ID_PREVIEW_BUTTON:
            PrintPreview(hwnd);
            break;
        case ID_SIMPAN_BUTTON:
            AddPenjualan(hwnd);
            break;
        case ID_KEMBALI_BUTTON:
            if (hwndTambahPenjualan != nullptr) {
                ShowWindow(hwndTambahPenjualan, SW_HIDE);
                SetFocus(hwndMainWindow);
            }
            break;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}