#include <iostream>
#include <vector>
#include <random>
#include <pthread.h>
#include <chrono>

struct KonvolusyonArgs {
    const std::vector<double>* x;
    const std::vector<double>* h;
    std::vector<double>* y;
    size_t baslangic_n;
    size_t bitis_n;// threadler arasında paylaşılan parametreler
};

void* Konvolusyon_thread(void* args) {
    KonvolusyonArgs* konv_args = static_cast<KonvolusyonArgs*>(args);
    const std::vector<double>& x = *konv_args->x;
    const std::vector<double>& h = *konv_args->h;
    std::vector<double>& y = *konv_args->y;
    for (size_t n = konv_args->baslangic_n; n < konv_args->bitis_n; ++n) {
        for (size_t k = 0; k < h.size(); ++k) {
            if (n >= k && n - k < x.size()) {
                y[n] += x[n - k] * h[k];
            }// bir thread'in konvolüsyonu belirli bir aralıkta gerçekleştirmesini sağlar
        }
    }
    return nullptr;
}

int main() {
    setlocale(LC_ALL, "Turkish");

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-5, 5);

    std::vector<double> x(100000);//M değeri    
    std::vector<double> h(10);// N değeri
    for (auto& xi : x) xi = dis(gen);
    for (auto& hi : h) hi = dis(gen);

    std::vector<double> y(x.size() + h.size() - 1, 0);

    size_t thread_sayisi = 8;// Thread sayısı
    std::vector<pthread_t> threads(thread_sayisi);
    std::vector<KonvolusyonArgs> thread_args(thread_sayisi);
    size_t n_bölü_thread = y.size() / thread_sayisi;

    auto baslangic_hesaplama = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < thread_sayisi; ++i) {
        thread_args[i] = { &x, &h, &y, i * n_bölü_thread, (i + 1) * n_bölü_thread };
        if (i == thread_sayisi - 1) thread_args[i].bitis_n = y.size();
        pthread_create(&threads[i], nullptr, Konvolusyon_thread, &thread_args[i]);
        //thread oluşturularak konvolüsyon işlemi paralelleşir
    }

    for (auto& thread : threads) pthread_join(thread, nullptr);// threadlein tamamlanmasını bekler.

    auto bitis_hesaplama = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> hesaplama_fark = bitis_hesaplama - baslangic_hesaplama;

    std::cout << "N: " << x.size() << ", M: " << h.size() << ", Thread Sayisi: " << thread_sayisi << ", Hesaplama Suresi: " << hesaplama_fark.count() << " saniye\n";
    std::cout << "Hesaplama İşlemi Tamamlandı!" << "\n";

    return 0;
}
