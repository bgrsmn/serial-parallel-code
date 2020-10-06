#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>  /* uintptr_t için */
#define HAVE_STRUCT_TIMESPEC //Başlık dosyasını düzenlemek için(Pthreads kapsamlı bir şekilde kullandığımız için)
#include <pthread.h> 
#include <chrono>
#include <iostream>
#include <omp.h>

using namespace std;

#define N 1000    //Belirlenen Boyut

#define NTHREAD 10 //N x N matrisinin satırlarını hesaplamak icin 4 thread kullan

  
#define MATRIS_ARALIK_BASINA_THREAD (N/NTHREAD) // Matrisi evre başına bir aralık düşücek şekilde ayırıyoruz.


float* matris_x[N]; //X matris vektörü
float* matris_y[N]; //Y matris vektörü

static long nokta_urun;

static pthread_mutex_t mutex_nokta_urunu; //Tüm evreler arasında paylaşılan karşılıklı dışlayıcı.

static void duzenleme(void) {
	
	//N x N uzunluğunda iki matrisi oluşturdum ve içerisini 1.0f şeklinde doldurdum. 
	for (long i = 0; i < N; i++) {
		matris_x[i] = (float*)malloc(N * sizeof(float)); //NxN matrisi icin her i satırına j = N kadar yer tahsis ettim
		matris_y[i] = (float*)malloc(N * sizeof(float)); //NxN matrisi icin her i satırına j = N kadar yer tahsis ettim
	}

	for (long i = 0; i < N; i++)
		for (long j = 0; j < N; j++) {
			matris_x[i][j] = matris_y[i][j] = 1.0f;
		}
}

//Çarpımı alıyoruz(Bir evre olarak)
#pragma warning(default:4716) //Derleyici uyarı iletilerinin davranışının seçmeli olarak değiştirilmesine izin vermez.
static void* carpim(void* dgr)
{

   //Evre oluşturulurken ilgili evre sırası bir void işaretçide taşınarak
   //bu işleve iletiliyor.  Sıra numarasını kullanarak vektörlerin hangi
   //aralığında çalışacağımızı belirleyeceğiz.Öncelikle void
   //işaretçiden pozitif tamsayıya döünştürmek için uygun bir tip
   //tanımlıyorum.

	uintptr_t offset;

	long i, j, t, baslar, biter;
	long topla;

	//Tip dönüşümü 4 kanallı çalışıyorsak offset değeri 0,1,2,3 şeklinde olmalıdır.
	
	offset = (uintptr_t) dgr;


	//Burdaki evrenin çalışacağı aralığı belirtir,başlangıç ve bitiş şeklinde indisler hesaplanır

	 baslar = offset * MATRIS_ARALIK_BASINA_THREAD;
	 biter = baslar + MATRIS_ARALIK_BASINA_THREAD;

	 //İşlem yapılır.Burada her evre kendi aralığında bağımsız şekilde
	 //çalışıyor.Örneğin elemanları çarp,topla ve ilgili aralığa yerleştir
	 

	for (i = baslar,topla = 0; i < biter; i++){  //Her thread icin baslangıc ve bitis satır numarası arasında döngü kurarız
		for ( j = 0; j < N; j++) {
			for ( t = 0; t < N; t++) {  //Her satır uzunlugu kadar döngü oluşur
			     topla += (matris_x[i][j] * matris_y[j][t]);
			}
		}
	}
	
	 //Burdaki evre bir sonuç üretti.Bu sonucu toplam sonuca
	 //ekleyeceğiz. Fakat aynı anda bir başka evre de bu işlemi yapıyor
	 //olabilir.  Dolayısıyla sonuç değişkenine yazarken diğer evrelerin
	 //bu değişkene erişmesini engellemeliyiz.
	 //Karşılıklı kilitlemeyi mümkün olan en dar aralıkta, yani
	 //kiritk bölge"de yapıyoruz.

	pthread_mutex_lock(&mutex_nokta_urunu);

	nokta_urun += topla;

	pthread_mutex_unlock(&mutex_nokta_urunu);

	 //Evre tamamlandı.pthread_join bu çağrıyı bekliyor.
	
	pthread_exit(NULL);
	
	return 0;
}  

//Verileri yok ediyoruz
static void silme(void)
{
	for (long i = 0; i < N; i++) {
		free(matris_x[i]);
		free(matris_y[i]);
	}
}

int main(void)
{
	
	//Ardından void işaretçiye dönüştürülecek bir pozitif tamsayı
	// gereklidir int veya uint kullanamıyoruz,çünkütaşınabilir değil ve uyarı
    // üretir.Bunun yerine stdint.h başlığında tanımlı olan özel bir
	// veritipi kullanıyorum.Bu sayede basitçe bir pozitif tamsayı 
	// sorunsuz şekilde void bir işaretçiye dönüştürülebilir hale gelmiş oluyor.

	double st = omp_get_wtime(); //Zaman ölçümü için kullandığım kod parçası

	uintptr_t i;

	pthread_t threads[NTHREAD]; //Evre Dizisi

	duzenleme(); //Matrisleri Oluşturma ve içerik düzenleme

	pthread_mutex_init(&mutex_nokta_urunu, NULL);

	//Evreleri oluşturup işlevlere bağlıyorum
	for (i = 0; i < NTHREAD; i++) {
		pthread_create(&threads[i], NULL, carpim, (void*)i);  //Tip dönüşümü önemli
	}
	//Evrelerin tamamlanmasını bekliyoruz.
	 
	 // Her evre kendi işlemini yapmış oluyor.Sonucu almam için katılımcı tüm
	 // evrelerin tamamlanmasını beklemeliyiz.Evreleri görevlendirildikleri
	 // işler için bir yere gönderilen ulaklar gibi düşünebiliriz.  Bu ulaklar
	 // görevleri tamamlandığında bir noktada buluşuyorlar.

	for (i = 0; i < NTHREAD; i++) {
		pthread_join(threads[i], NULL);
	}

	silme(); //Oluşturulan matrisler silindi

	pthread_mutex_destroy(&mutex_nokta_urunu); //Karşılıklı dışlayıcı silindi


	double en = omp_get_wtime(); //Zaman ölçümü tamamlanır
	cout << "Paralel Zaman:" << en - st; //Ölçülen zaman yazdırılır

	exit(EXIT_SUCCESS); //Program sonlandırıldı
}
