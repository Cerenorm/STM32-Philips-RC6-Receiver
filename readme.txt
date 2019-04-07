Bu program CoIDE 1.7.8 ortamında stm32f103 mikrodenetleyici ile rc6 tipi uzaktan kumanda sinyalini almak için yazılmıştır. 
Donanım olrak IR alıcı, stm32f103 geliştime kartı ve 128x64 oled ekran kullanılmıştır. Programın çalışması HP rc6 kumanda 
ile test edilmiştir. Program Philips rc6 protokolünü de destekler. Program içinde yapılacak küçük değişikliklerle farklı
mikrodenetleyicilerde çalışabilir. Bunun için rc6.h ve rc6.c dosyalarını projenize dahil etmeniz gerekiyor. Daha sonra rc6.c
dosyasından giriş portu ve kesme fonksiyonunu kullandığınız mikrodenetleyiciye uygun olarak değiştirin. 
Sonuçları görüntülemek için kullanılan ssd1306 oled ekran sürücüsü hazır kullanılmıştır yalnızca 16x16 font eklenmiştir.
