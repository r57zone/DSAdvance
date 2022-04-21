[![EN](https://user-images.githubusercontent.com/9499881/33184537-7be87e86-d096-11e7-89bb-f3286f752bc6.png)](https://github.com/r57zone/DSAdvance/) 
[![RU](https://user-images.githubusercontent.com/9499881/27683795-5b0fbac6-5cd8-11e7-929c-057833e01fb1.png)](https://github.com/r57zone/DSAdvance/blob/master/README.RU.md)

# DSAdvance
Продвинутая эмуляция Xbox геймпада для Sony DualSense и DualShock 4. Поддерживаются прицеливание и вождение наклонами геймпада, а также эмуляция стиков на тачпаде. Работает на базе драйвера [ViGEm](https://github.com/ViGEm).

# Режимы работы
Поддерживается несколько режимов работы, переключаются они нажатиями по тачпаду. Яркость регулируется без нажатия. 

![](https://user-images.githubusercontent.com/9499881/164546701-c1f49c86-2f65-45b0-9a8f-83d751b46004.png)

Для того, чтобы выйти из режима эмуляции стиков на тачпаде нужно переключиться на режим по умолчанию.


Кнопка "PS" симулирует нажатие "Win" + "G", а кнопка микрофона "Win" + "Alt" + "PrtScr".

## Настройка
1. Установить [ViGEmBus](https://github.com/ViGEm/ViGEmBus/releases).
2. Установите Microsoft Visual C++ Redistributable 2017 или новее.
3. Подключить геймпад Sony DualSense или DualShock 4 по USB (беспроводной режим пока не поддерживается).
4. Распаковать и запустить DSAdvance.
5. При необходимости измените мёртвые зоны стиков или другие параметры, в конфигурационном файле "Config.ini".

## Загрузка
>Версия для Windows 10.

**[Загрузить](https://github.com/r57zone/DSAdvance/releases)**

## Благодарности
* [ViGEm](https://github.com/ViGEm) за возможность эмуляции разных геймпадов.
* [HIDAPI library](https://github.com/signal11/hidapi), с [исправлениями](https://github.com/libusb/hidapi), за библиотеку для работы с USB устройства. В проекте используется этот [форк](https://github.com/r57zone/hidapi).
* [JoyShockLibrary](https://github.com/JibbSmart/JoyShockLibrary) за классную библиотеку геймпадов, позволяющую легко получить вращение контроллера.
* Разработчику [DualSense4Windows](https://github.com/broken-bytes/DualSense4Windows) & [пользователям Reddit](https://www.reddit.com/r/gamedev/comments/jumvi5/dualsense_haptics_leds_and_more_hid_output_report/) за детальное описание выходного USB пакета.
* Разработчику [Gen_Dev_TactonBiotic](https://github.com/hizbi-github/Gen_Dev_TactonBiotic) за пример работы с JoyShockLibrary.

## Обратная связь
`r57zone[собака]gmail.com`