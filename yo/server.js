/*
=============================================================================
     DYNAMIC BACKEND SERVER - HE THONG QUAN LY THU VIEN (NODE.JS)
=============================================================================
 - Dong bo du lieu 2 chieu voi file data.txt va phieu_muon.txt
 - Cung cap RESTful API cho Web Dong: Them, Xoa, Muon, Tra, Queue, Undo
 - Phuc vu giao dien Web thong_ke.html
=============================================================================
*/

const http = require('http');
const fs = require('fs');
const path = require('path');
const url = require('url');

const PORT = 8080;
const DATA_FILE = path.join(__dirname, 'data.txt');
const BORROW_FILE = path.join(__dirname, 'phieu_muon.txt');

// Hang doi dat truoc sach (Queue FIFO trong bo nho)
let reservationQueue = [];

// Ngan xep hoan tac (Stack LIFO trong bo nho)
let undoStack = [];

// =============================================================================
// HAM DOC / GHI FILE
// =============================================================================

function readBooks() {
    if (!fs.existsSync(DATA_FILE)) return [];
    const content = fs.readFileSync(DATA_FILE, 'utf-8');
    const lines = content.split(/\r?\n/).filter(line => line.trim().length > 0);
    return lines.map(line => {
        const parts = line.split('|');
        return {
            maSach: parts[0] || '',
            tenSach: parts[1] || '',
            loaiSach: parts[2] || 'Khac',
            tongSoLuong: parseInt(parts[3]) || 0,
            soLuongConLai: parseInt(parts[4]) || 0,
            luotMuon: parseInt(parts[5]) || 0
        };
    });
}

function saveBooks(books) {
    const lines = books.map(b => 
        `${b.maSach}|${b.tenSach}|${b.loaiSach}|${b.tongSoLuong}|${b.soLuongConLai}|${b.luotMuon}`
    );
    fs.writeFileSync(DATA_FILE, lines.join('\n') + '\n', 'utf-8');
}

function readBorrowers() {
    if (!fs.existsSync(BORROW_FILE)) return [];
    const content = fs.readFileSync(BORROW_FILE, 'utf-8');
    const lines = content.split(/\r?\n/).filter(line => line.trim().length > 0);
    return lines.map(line => {
        const parts = line.split('|');
        return {
            maPhieu: parts[0] || '',
            tenSinhVien: parts[1] || '',
            maSinhVien: parts[2] || '',
            maSach: parts[3] || '',
            tenSach: parts[4] || '',
            loaiSach: parts[5] || '',
            ngayMuon: parts[6] || ''
        };
    });
}

function saveBorrowers(borrowers) {
    const lines = borrowers.map(b => 
        `${b.maPhieu}|${b.tenSinhVien}|${b.maSinhVien}|${b.maSach}|${b.tenSach}|${b.loaiSach}|${b.ngayMuon}`
    );
    fs.writeFileSync(BORROW_FILE, lines.join('\n') + '\n', 'utf-8');
}

// =============================================================================
// TIEN ICH HTTP
// =============================================================================

function sendJSON(res, statusCode, data) {
    res.writeHead(statusCode, {
        'Content-Type': 'application/json; charset=utf-8',
        'Access-Control-Allow-Origin': '*',
        'Access-Control-Allow-Methods': 'GET, POST, DELETE, OPTIONS',
        'Access-Control-Allow-Headers': 'Content-Type'
    });
    res.end(JSON.stringify(data));
}

function getRequestBody(req) {
    return new Promise((resolve, reject) => {
        let body = '';
        req.on('data', chunk => { body += chunk.toString(); });
        req.on('end', () => {
            try {
                resolve(body ? JSON.parse(body) : {});
            } catch (err) {
                reject(err);
            }
        });
        req.on('error', err => reject(err));
    });
}

// =============================================================================
// SERVER HTTP & ROUTER
// =============================================================================

const server = http.createServer(async (req, res) => {
    // Xu ly CORS pre-flight
    if (req.method === 'OPTIONS') {
        res.writeHead(204, {
            'Access-Control-Allow-Origin': '*',
            'Access-Control-Allow-Methods': 'GET, POST, DELETE, OPTIONS',
            'Access-Control-Allow-Headers': 'Content-Type'
        });
        res.end();
        return;
    }

    const parsedUrl = url.parse(req.url, true);
    const pathname = parsedUrl.pathname;
    const method = req.method;

    // -------------------------------------------------------------------------
    // API ENDPOINTS
    // -------------------------------------------------------------------------

    // 1. GET /api/stats - Thong ke tong quan
    if (pathname === '/api/stats' && method === 'GET') {
        const books = readBooks();
        const borrowers = readBorrowers();
        const tongDau = books.length;
        const tongKho = books.reduce((sum, b) => sum + b.tongSoLuong, 0);
        const tongCon = books.reduce((sum, b) => sum + b.soLuongConLai, 0);
        const tongMuon = borrowers.length;
        const tongLuot = books.reduce((sum, b) => sum + b.luotMuon, 0);

        return sendJSON(res, 200, {
            success: true,
            data: {
                tongDauSach: tongDau,
                tongSoLuongKho: tongKho,
                soLuongConLai: tongCon,
                soSachDangMuon: tongMuon,
                tongLuotMuon: tongLuot,
                soNguoiDatCho: reservationQueue.length,
                soThaoTacUndo: undoStack.length
            }
        });
    }

    // 2. GET /api/books - Danh sach sach
    if (pathname === '/api/books' && method === 'GET') {
        let books = readBooks();
        const search = (parsedUrl.query.search || '').trim().toLowerCase();
        const category = (parsedUrl.query.category || '').trim();

        if (search) {
            books = books.filter(b => 
                b.maSach.toLowerCase().includes(search) || 
                b.tenSach.toLowerCase().includes(search) ||
                b.loaiSach.toLowerCase().includes(search)
            );
        }
        if (category && category !== 'all') {
            if (category === 'avail') books = books.filter(b => b.soLuongConLai > 0);
            else if (category === 'out') books = books.filter(b => b.soLuongConLai <= 0);
            else books = books.filter(b => b.loaiSach.toLowerCase() === category.toLowerCase());
        }

        return sendJSON(res, 200, { success: true, count: books.length, data: books });
    }

    // 3. POST /api/books - Them sach moi
    if (pathname === '/api/books' && method === 'POST') {
        try {
            const body = await getRequestBody(req);
            const { maSach, tenSach, loaiSach, tongSoLuong } = body;

            if (!maSach || !tenSach) {
                return sendJSON(res, 400, { success: false, message: 'Ma sach va Ten sach la bat buoc!' });
            }

            const books = readBooks();
            if (books.some(b => b.maSach.toLowerCase() === maSach.trim().toLowerCase())) {
                return sendJSON(res, 400, { success: false, message: `Ma sach '${maSach}' da ton tai!` });
            }

            const sl = parseInt(tongSoLuong) || 5;
            const newBook = {
                maSach: maSach.trim().toUpperCase(),
                tenSach: tenSach.trim(),
                loaiSach: (loaiSach || 'Tham khao').trim(),
                tongSoLuong: sl,
                soLuongConLai: sl,
                luotMuon: 0
            };

            books.push(newBook);
            saveBooks(books);

            return sendJSON(res, 201, { success: true, message: `Da them dau sach '${newBook.tenSach}' thanh cong!`, data: newBook });
        } catch (err) {
            return sendJSON(res, 500, { success: false, message: err.message });
        }
    }

    // 4. DELETE /api/books/:maSach - Xoa sach
    if (pathname.startsWith('/api/books/') && method === 'DELETE') {
        const maSach = decodeURIComponent(pathname.replace('/api/books/', '')).trim();
        const books = readBooks();
        const idx = books.findIndex(b => b.maSach.toLowerCase() === maSach.toLowerCase());

        if (idx === -1) {
            return sendJSON(res, 404, { success: false, message: `Khong tim thay sach co ma '${maSach}'!` });
        }

        const deletedBook = books.splice(idx, 1)[0];
        saveBooks(books);

        undoStack.push({ type: 'DELETE_BOOK', book: deletedBook });
        return sendJSON(res, 200, { success: true, message: `Da xoa dau sach '${deletedBook.tenSach}'!`, data: deletedBook });
    }

    // 5. GET /api/borrowers - Danh sach sinh vien dang muon
    if (pathname === '/api/borrowers' && method === 'GET') {
        const borrowers = readBorrowers();
        return sendJSON(res, 200, { success: true, count: borrowers.length, data: borrowers });
    }

    // 6. POST /api/borrow - Cho sinh vien muon sach
    if (pathname === '/api/borrow' && method === 'POST') {
        try {
            const body = await getRequestBody(req);
            const { maSach, tenSinhVien, maSinhVien, ngayMuon } = body;

            if (!maSach || !tenSinhVien || !maSinhVien) {
                return sendJSON(res, 400, { success: false, message: 'Vui long nhap day du Ma sach, Ho ten SV va MSSV!' });
            }

            const books = readBooks();
            const book = books.find(b => b.maSach.toLowerCase() === maSach.trim().toLowerCase());

            if (!book) {
                return sendJSON(res, 404, { success: false, message: `Khong tim thay dau sach co ma '${maSach}'!` });
            }

            if (book.soLuongConLai <= 0) {
                return sendJSON(res, 409, {
                    success: false,
                    isOutOfStock: true,
                    message: `Cuon sach '${book.tenSach}' da het trong kho! Ban co muon xep vao Hang doi dat truoc?`
                });
            }

            // Giam so luong con lai va tang luot muon
            book.soLuongConLai--;
            book.luotMuon++;
            saveBooks(books);

            // Tao ma phieu muon tu dong
            const borrowers = readBorrowers();
            const nextNum = borrowers.length + 1;
            const maPhieu = `PM${String(nextNum).padStart(3, '0')}`;
            const dateStr = ngayMuon || new Date().toLocaleDateString('vi-VN');

            const newTicket = {
                maPhieu,
                tenSinhVien: tenSinhVien.trim(),
                maSinhVien: maSinhVien.trim(),
                maSach: book.maSach,
                tenSach: book.tenSach,
                loaiSach: book.loaiSach,
                ngayMuon: dateStr
            };

            borrowers.unshift(newTicket);
            saveBorrowers(borrowers);

            // Luu vao Stack Undo
            undoStack.push({ type: 'BORROW', ticket: newTicket });

            return sendJSON(res, 200, {
                success: true,
                message: `Lap phieu muon ${maPhieu} cho SV ${newTicket.tenSinhVien} muon '${book.tenSach}' thanh cong!`,
                data: { ticket: newTicket, book }
            });
        } catch (err) {
            return sendJSON(res, 500, { success: false, message: err.message });
        }
    }

    // 7. POST /api/return - Nhan tra sach
    if (pathname === '/api/return' && method === 'POST') {
        try {
            const body = await getRequestBody(req);
            const { maPhieu, maSinhVien, maSach } = body;

            const borrowers = readBorrowers();
            let idx = -1;

            if (maPhieu) {
                idx = borrowers.findIndex(p => p.maPhieu.toLowerCase() === maPhieu.trim().toLowerCase());
            } else if (maSinhVien && maSach) {
                idx = borrowers.findIndex(p => 
                    p.maSinhVien.toLowerCase() === maSinhVien.trim().toLowerCase() && 
                    p.maSach.toLowerCase() === maSach.trim().toLowerCase()
                );
            }

            if (idx === -1) {
                return sendJSON(res, 404, { success: false, message: 'Khong tim thay phieu muon phu hop!' });
            }

            const returnedTicket = borrowers.splice(idx, 1)[0];
            saveBorrowers(borrowers);

            // Tang lai so luong sach
            const books = readBooks();
            const book = books.find(b => b.maSach.toLowerCase() === returnedTicket.maSach.toLowerCase());
            if (book && book.soLuongConLai < book.tongSoLuong) {
                book.soLuongConLai++;
                saveBooks(books);
            }

            undoStack.push({ type: 'RETURN', ticket: returnedTicket });

            // Kiem tra co ai trong Hang doi dang cho sach nay khong
            let priorityStudent = null;
            const qIdx = reservationQueue.findIndex(q => q.maSachCanMuon.toLowerCase() === returnedTicket.maSach.toLowerCase());
            if (qIdx !== -1) {
                priorityStudent = reservationQueue[qIdx];
            }

            return sendJSON(res, 200, {
                success: true,
                message: `Da nhan tra sach '${returnedTicket.tenSach}' tu SV ${returnedTicket.tenSinhVien}!`,
                priorityStudent: priorityStudent ? `Sinh vien ${priorityStudent.tenSinhVien} (MSSV: ${priorityStudent.maSinhVien}) dang cho muon sach nay!` : null
            });
        } catch (err) {
            return sendJSON(res, 500, { success: false, message: err.message });
        }
    }

    // 8. GET & POST /api/queue - Hang doi dat truoc
    if (pathname === '/api/queue' && method === 'GET') {
        return sendJSON(res, 200, { success: true, count: reservationQueue.length, data: reservationQueue });
    }

    if (pathname === '/api/queue' && method === 'POST') {
        try {
            const body = await getRequestBody(req);
            const { tenSinhVien, maSinhVien, maSach } = body;

            if (!tenSinhVien || !maSinhVien || !maSach) {
                return sendJSON(res, 400, { success: false, message: 'Thieu thong tin de xep hang doi!' });
            }

            const item = {
                id: Date.now(),
                tenSinhVien: tenSinhVien.trim(),
                maSinhVien: maSinhVien.trim(),
                maSachCanMuon: maSach.trim(),
                ngayDat: new Date().toLocaleDateString('vi-VN')
            };

            reservationQueue.push(item);
            return sendJSON(res, 201, {
                success: true,
                message: `Sinh vien ${item.tenSinhVien} (MSSV: ${item.maSinhVien}) da duoc xep vao Hang doi tai vi tri thu ${reservationQueue.length}!`,
                position: reservationQueue.length
            });
        } catch (err) {
            return sendJSON(res, 500, { success: false, message: err.message });
        }
    }

    // 9. POST /api/queue/dequeue - Phuc vu nguoi dau hang doi
    if (pathname === '/api/queue/dequeue' && method === 'POST') {
        if (reservationQueue.length === 0) {
            return sendJSON(res, 400, { success: false, message: 'Hang doi dat cho hien dang trong!' });
        }

        const nextPerson = reservationQueue.shift();
        return sendJSON(res, 200, {
            success: true,
            message: `Da moi sinh vien ${nextPerson.tenSinhVien} (MSSV: ${nextPerson.maSinhVien}) lam thu tuc muon sach!`,
            data: nextPerson
        });
    }

    // 10. GET & POST /api/undo - Hoan tac (Stack LIFO)
    if (pathname === '/api/undo' && method === 'GET') {
        return sendJSON(res, 200, { success: true, count: undoStack.length });
    }

    if (pathname === '/api/undo' && method === 'POST') {
        if (undoStack.length === 0) {
            return sendJSON(res, 400, { success: false, message: 'Ngan xep Undo dang rong, khong co thao tac nao de hoan tac!' });
        }

        const act = undoStack.pop();
        const books = readBooks();
        const borrowers = readBorrowers();

        if (act.type === 'BORROW') {
            // Hoan tac muon: xoa phieu muon khoi danh sach, tra lai so luong sach
            const pIdx = borrowers.findIndex(p => p.maPhieu === act.ticket.maPhieu);
            if (pIdx !== -1) borrowers.splice(pIdx, 1);
            saveBorrowers(borrowers);

            const b = books.find(item => item.maSach.toLowerCase() === act.ticket.maSach.toLowerCase());
            if (b) {
                b.soLuongConLai++;
                b.luotMuon = Math.max(0, b.luotMuon - 1);
                saveBooks(books);
            }
            return sendJSON(res, 200, {
                success: true,
                message: `[Undo] Da huy phieu muon cua SV ${act.ticket.tenSinhVien}, tra lai so luong cho sach '${act.ticket.tenSach}'.`
            });
        } 
        else if (act.type === 'RETURN') {
            // Hoan tac tra: them lai phieu muon, giam so luong sach
            borrowers.unshift(act.ticket);
            saveBorrowers(borrowers);

            const b = books.find(item => item.maSach.toLowerCase() === act.ticket.maSach.toLowerCase());
            if (b && b.soLuongConLai > 0) {
                b.soLuongConLai--;
                saveBooks(books);
            }
            return sendJSON(res, 200, {
                success: true,
                message: `[Undo] Da khoi phuc trang thai muon cho SV ${act.ticket.tenSinhVien} doi voi sach '${act.ticket.tenSach}'.`
            });
        }
        else if (act.type === 'DELETE_BOOK') {
            books.push(act.book);
            saveBooks(books);
            return sendJSON(res, 200, {
                success: true,
                message: `[Undo] Da phuc hoi dau sach '${act.book.tenSach}' vao thu vien.`
            });
        }
    }

    // -------------------------------------------------------------------------
    // SERVE STATIC FILES
    // -------------------------------------------------------------------------

    let filePath = path.join(__dirname, pathname === '/' ? 'thong_ke.html' : pathname);

    // Kiem tra file ton tai
    if (fs.existsSync(filePath) && fs.statSync(filePath).isFile()) {
        const ext = path.extname(filePath).toLowerCase();
        const mimeTypes = {
            '.html': 'text/html; charset=utf-8',
            '.css': 'text/css; charset=utf-8',
            '.js': 'application/javascript; charset=utf-8',
            '.json': 'application/json; charset=utf-8',
            '.pdf': 'application/pdf',
            '.txt': 'text/plain; charset=utf-8',
            '.ico': 'image/x-icon'
        };

        const contentType = mimeTypes[ext] || 'application/octet-stream';
        res.writeHead(200, { 'Content-Type': contentType });
        fs.createReadStream(filePath).pipe(res);
        return;
    }

    // 404 Not Found
    res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
    res.end('404 Not Found');
});

server.listen(PORT, () => {
    console.log(`=============================================================`);
    console.log(`  CONG THONG TIN THU VIEN DONG (DYNAMIC WEB SERVER RUNNING)  `);
    console.log(`=============================================================`);
    console.log(`  Dia chi Web: http://localhost:${PORT}`);
    console.log(`  File du lieu: ${DATA_FILE}`);
    console.log(`  File phieu muon: ${BORROW_FILE}`);
    console.log(`=============================================================`);
});
