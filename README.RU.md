[![EN](https://user-images.githubusercontent.com/9499881/33184537-7be87e86-d096-11e7-89bb-f3286f752bc6.png)](https://github.com/r57zone/DSAdvance/) 
[![RU](https://user-images.githubusercontent.com/9499881/27683795-5b0fbac6-5cd8-11e7-929c-057833e01fb1.png)](https://github.com/r57zone/DSAdvance/blob/master/README.RU.md)

# DSAdvance
Продвинутая эмуляция Xbox геймпада для Sony DualSense, DualShock 4, Nintendo Pro контроллера или Joycon-ов. Поддерживается прицеливание и вождение наклонами геймпада, а также эмуляция стиков на тачпаде. Работает на базе драйвера [ViGEm](https://github.com/ViGEm).

[![](https://user-images.githubusercontent.com/9499881/164945071-5b9f86dd-c396-45a5-817b-fc7068450f02.gif)](https://youtu.be/gkyqO_HuPnk)
[![](https://user-images.githubusercontent.com/9499881/164945073-cfa1bfb7-cb82-4714-b2ad-7ecd84a5bcfc.gif)](https://youtu.be/gkyqO_HuPnk)

# Режимы работы
Поддерживается несколько режимов работы, переключаются они нажатиями по тачпаду для DualSene и DualShock 4, а для Pro контроллеров, и JoyCon-ов на кнопки `+`, и `-`.

![](https://user-images.githubusercontent.com/9499881/173076125-b3762211-74ab-4377-a6a2-a7b6c9b1a142.png)

Для того, чтобы выйти из режима эмуляции стиков на тачпаде нужно переключиться на режим по умолчанию. 


Нажимая на профиль по умолчанию, на DualSense, белые светодиоды отображают текущий статус заряда аккумулятора (1 - 25%, 2 - 50%, 3 - 75%, 4 - 100%).


Для изменения яркости 2 раза нажмите на область яркости. Если изменение яркости заблокировано, то подстветка будет выключаться по двойному клику.


Кнопка `PS` симулирует нажатие `Win+G`, а кнопка микрофона `Win+Alt+PrtScr`.

## Настройка
1. Установить [ViGEmBus](https://github.com/ViGEm/ViGEmBus/releases).
2. Установите Microsoft Visual C++ Redistributable 2017 или новее.
3. Подключить геймпад Sony DualSense, DualShock 4, Nintendo Pro controller или JoyCon-ы (рекомендуется по проводу, прицеливание работает лучше).
4. Распаковать и запустить DSAdvance.
5. При необходимости измените мёртвые зоны стиков или другие параметры, в конфигурационном файле `Config.ini`.
6. При использовании со Steam играми, в настройках контроллера, отключите "персональные настройки Playstation".


## Возможные проблемы
• **Постоянное изменение значков клавиатуры и геймпада**<br>
Если это мешает вам, то измените режим прицеливания на "mouse-joystick", в программе или используйте прицеливание, с помощью левого триггера.



• **Не работает вождение, в играх с поддержкой DualSense**<br>
Сначала запустите DSAdvance, а только потом саму игру, игра может отдать приоритет эмулируемому геймпаду Xbox и вождение будет работать. Также можно включить режим эмуляции "Only driving & aiming", чтобы геймпад включался только в режиме вождения.

## Загрузка
>Версия для Windows 10.

**[Загрузить](https://github.com/r57zone/DSAdvance/releases)**

## Благодарности
* Sony за самые продвинутые геймпады и инвестирование в инновации, а также Nintendo за продвижение подобных инноваций в игры.
* [ViGEm](https://github.com/ViGEm) за возможность эмуляции разных геймпадов.
* [HIDAPI library](https://github.com/signal11/hidapi), с [исправлениями](https://github.com/libusb/hidapi), за библиотеку для работы с USB устройства. В проекте используется этот [форк](https://github.com/r57zone/hidapi).
* [JoyShockLibrary](https://github.com/JibbSmart/JoyShockLibrary) за классную библиотеку геймпадов, позволяющую легко получить вращение контроллера. Также используется некоторый код из этой библиотеки.
* [Пользователям Reddit](https://www.reddit.com/r/gamedev/comments/jumvi5/dualsense_haptics_leds_and_more_hid_output_report/) за детальное описание выходного USB пакета.

## Обратная связь
`r57zone[собака]gmail.com`