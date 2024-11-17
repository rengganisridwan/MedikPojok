# Panduan Penggunaan BioAmp EXG Pill

## Pendahuluan

![[img/bitmap.png]]

BioAmp EXG Pill merupakan bioamplifier buatan Upside Down Labs dari India yang dapat digunakan untuk sinyal biopotensial seperti EEG, EKG, EMG, dan EOG. Dengan daya dan *noise* yang rendah serta ukurannya yang kecil, bioamplifier ini dapat dengan mudah diintegrasikan ke dalam berbagai sistem melalui unit mikrokontroler 5 V (contoh: Arduino Uno) yang dilengkapi dengan *analog-to-digital converter* (ADC). Karena bioamplifier ini bersifat *open-source*, publik dapat dengan bebas mengakses informasi seperti skematik rangkaian dan contoh kode program dari perangkat ini melalui [GitHub](https://github.com/upsidedownlabs/BioAmp-EXG-Pill) dan [*website*](https://docs.upsidedownlabs.tech/hardware/bioamp/bioamp-exg-pill/index.html) mereka.

## Konfigurasi

![[img/v1.0_Back_Specifications.jpg]]

Bioamplifier ini memiliki total 6 pin yang terbagi menjadi 2 kelompok:
- Kelompok pin pertama (kiri pada gambar) terdiri dari 3 pin pada konektor JST yang terhubung ke kancing elektroda.
	- `REF` atau DRL (*driven right leg*), dihubungkan ke pin *reference* pada elektroda.
	- `IN+` atau Input+, dihubungkan ke pin positif elektroda.
	- `IN-` atau Input-, dihubungkan ke pin negatif elektroda.
- Kelompok pin kedua (kanan pada gambar) terdiri dari 3 pin *male* yang terhubung ke mikrokontroler.
	- `OUT` atau Output, merupakan pin yang menghasilkan sinyal analog dari pengukuran sinyal biopotensial dengan rentang nilai 0-5 V.
	- `GND` atau Ground, pin yang dihubungkan ke pin GND pada mikrokontroler.
	- `VCC` atau +5V, pin yang dihubungkan ke sumber tegangan 5 V yang tersedia pada mikrokontroler atau sumber eksternal.
	> [!WARNING]
	> Jika bioamplifier ini ingin digunakan pada mikrokontroler dengan level tegangan 3.3 V, sinyal analog pada pin `OUT` bioamplifier perlu diturunkan ke level tegangan 3.3 V dengan menggunakan rangkaian pembagi tegangan.

Terdapat juga beberapa *pad* pada bioamplifier yang dapat di-*short* atau ditambah dengan komponen lain untuk mengatur parameter seperti konfigurasi pengukuran (unipolar atau bipolar), besaran *gain*, dan karakteristik filter. Untuk kepraktisan penggunaan, fitur ini untuk sementara tidak digunakan. Jika parameter-parameter tersebut ingin diubah, pengubahan nilai parameter dapat dilakukan dengan menggunakan program.

## Prosedur Penggunaan dengan Arduino

![[img/Basic-Circuit.jpg]]

1. Sambungkan bioamplifier dengan kabel elektroda melalui konektor JST yang tersedia.

	| BioAmp EXG Pill | Elektroda |
	| --- | --- |
	| REF | Reference (kuning) |
	| IN+ | Positive (merah) |
	| IN- | Negative (hitam) |

2. Sambungkan bioamplifier dengan papan Arduino menggunakan kabel *jumper*.

	| BioAmp EXG Pill | Arduino |
	| --- | --- |
	| VCC | 5V |
	| GND | GND |
	| OUT | A0 |
	
	>[!IMPORTANT]
	> Pastikan bahwa pin VCC dan GND tidak terbalik. Pemasangan kabel *jumper* yang terbalik pada kedua pin tersebut dapat merusak bioamplifier.

3. *Upload* program ke Arduino menggunakan Arduino IDE.
4. Nyalakan Arduino dan buka *serial plotter* di Arduino IDE.
	
	> [!TIP]
	> Untuk mengetes apakah ketiga elektroda sudah siap digunakan untuk mengukur sinyal EMG, sentuhkan ketiga kepala elektroda. Elektroda telah siap digunakan jika sinyal yang muncul pada *serial plotter* menjadi datar (atau relatif datar) ketika ketiga kepala elektroda saling bersentuhan.

5. Hubungkan ketiga kabel elektroda dengan elektroda Ag/AgCl.
6. Bersihkan permukaan kulit yang akan ditempeli elektroda dengan menggunakan alkohol dan biarkan hingga kering. Hal ini dilakukan untuk membersihkan kulit dari kotoran, sel kulit mati, dan keringat.

![[img/EMG.jpg]]

7. Tempelkan ketiga elektroda ke permukaan kulit yang dekat dengan otot yang ingin diamati. Elektroda positif dan negatif perlu ditempelkan berdekatan satu sama lain pada bagian otot yang aktif ketika subjek melakukan gerakan yang ingin diukur, sedangkan elektroda referensi ditempelkan pada otot yang tidak aktif pada gerakan tersebut. 

>[!NOTE]
> Untuk mengukur kekuatan menggenggam, elektroda positif dan negatif dapat ditempelkan pada permukaan kulit yang dekat dengan otot flexor carpi ulnaris. Elektroda referensi dapat ditempelkan pada tulang siku (direkomendasikan) atau punggung tangan.
> 
> ![[img/0_AufUm_uN5DoqKoev.jpg]]
> (Sumber gambar: [link](https://medium.com/performance-course/the-importance-of-a-strong-grip-91a592323349))
