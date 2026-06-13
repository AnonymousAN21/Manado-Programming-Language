# Dokumentasi Teknis Bahasa Pemrograman MANADO

## Pengantar
Bahasa Pemrograman MANADO adalah bahasa pemrograman prosedural yang dirancang menggunakan sintaks berbasis bahasa Indonesia. Dokumentasi ini memberikan panduan teknis mengenai aturan penulisan, kata kunci, dan struktur dasar untuk membangun program menggunakan bahasa ini.

## Struktur Program dan Fungsi
Setiap eksekusi program selalu bermula dari titik awal yang secara spesifik dideklarasikan sebagai fungsi utama(). Pembuatan fungsi baru dilakukan dengan menulis awalan kata kunci fungsi yang langsung diikuti oleh nama fungsi beserta parameternya. Eksekusi atau pemanggilan fungsi kustom di dalam alur program menggunakan perintah panggil sebelum penulisan nama fungsi. Jika sebuah fungsi dirancang untuk melempar nilai hasil proses, pengembang harus menggunakan perintah kembalikan. Setiap blok deklarasi fungsi wajib diakhiri dengan penutup kelar.

Contoh implementasi fungsi:
```text
fungsi faktorial(n)
    kalo n lebih_kecil_sama_dengan 1 maka
        kembalikan 1
    kelar
    kembalikan n kali panggil faktorial(n kurang 1)
kelar

fungsi utama()
    angka hasil = panggil faktorial(5)
kelar
```

## Variabel dan Tipe Data
Bahasa ini menyederhanakan alokasi memori variabel dengan membatasi penggunaan pada satu deklarasi utama, yaitu angka. Perintah angka mendefinisikan nilai numerik bilangan bulat maupun kumpulan nilai bertipe array. Pembuatan koleksi data array dilakukan dengan menempatkan nilai-nilai ke dalam tanda kurung siku, sementara pemanggilan spesifik elemen array menggunakan indeks yang juga diletakkan di dalam tanda kurung siku.

Contoh deklarasi variabel dan array:
```text
angka a = 0
angka pilihan = 0
angka data = [5, 2, 9, 1, 7, 3, 8]
```
## Operator Sintaksis
Seluruh operasi komputasi diterjemahkan ke dalam representasi teks bahasa Indonesia. Perhitungan aritmatika dijalankan menggunakan perintah tambah, kurang, kali, dan bagi. Bahasa ini juga menyediakan perintah singkat kurang_satu untuk menurunkan nilai variabel secara bertahap dengan selisih satu. Evaluasi ekspresi perbandingan menggunakan penanda relasional sama_dengan, lebih_besar, lebih_kecil_sama_dengan, serta lebih_besar_sama_dengan. Penggabungan dua kondisi logika secara bersamaan dikelola oleh operator konjungsi deng.

```text
hasil = a tambah b
hasil = a kali b
j kurang_satu
kalo n lebih_kecil_sama_dengan 1 maka
ketika j lebih_besar_sama_dengan 0 deng data[j] lebih_besar key beking
```

## Input dan Output (I/O)
Mekanisme interaksi program dengan pengguna dikendalikan melalui dua instruksi absolut. Proses mencetak data ke layar menggunakan perintah tampilkan(...) yang mampu memproses teks literal dengan tanda kutip maupun mencetak isi variabel secara transparan. Proses perekaman data dari pengguna menggunakan instruksi masukkan yang langsung merujuk pada nama variabel penampung.

Contoh interaksi I/O:
```text
tampilkan("Masukkan angka pertama:")
masukkan a
tampilkan("Hasil:")
tampilkan(hasil)
```

## Struktur Kontrol Percabangan
Logika pengambilan keputusan diimplementasikan menggunakan blok kondisional linier. Blok evaluasi utama dibuka dengan instruksi kalo, dilanjutkan dengan kondisi yang ingin dievaluasi, dan diikat oleh perintah maka untuk memulai baris eksekusi. Kondisi pengecualian atau alternatif menggunakan blok nda untuk menangkap kondisi yang tidak memenuhi syarat awal. Seluruh alur kondisional ini ditutup sepenuhnya oleh instruksi kelar.

Contoh kontrol percabangan bersarang:
```text
kalo pilihan sama_dengan 1 maka
    hasil = a tambah b
nda
    kalo pilihan sama_dengan 2 maka
        hasil = a kurang b
    nda
        tampilkan("Pilihan tidak valid!")
    kelar
kelar
```

## Struktur Iterasi
Bahasa ini mendistribusikan proses perulangan ke dalam dua kerangka logika operasional. Iterasi terukur yang mengandalkan batasan awal dan akhir ditulis menggunakan struktur dari nilai_awal sampe nilai_akhir beking. Iterasi tidak terukur yang berjalan selama syarat terpenuhi ditulis menggunakan format ketika kondisi_berjalan beking. Semua rentetan perulangan tersebut harus diputus garis eksekusinya menggunakan instruksi penutup kelar.

Contoh perulangan terukur (For Loop):

```text
dari i = 0 sampe n kurang 1 beking
    tampilkan(data[i])
kelar
```

```text
ketika j lebih_besar_sama_dengan 0 deng data[j] lebih_besar key beking
    data[j tambah 1] = data[j]
    j kurang_satu
kelar
```