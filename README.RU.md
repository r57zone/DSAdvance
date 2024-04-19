[![EN](https://user-images.githubusercontent.com/9499881/33184537-7be87e86-d096-11e7-89bb-f3286f752bc6.png)](https://github.com/r57zone/DSAdvance/) 
[![RU](https://user-images.githubusercontent.com/9499881/27683795-5b0fbac6-5cd8-11e7-929c-057833e01fb1.png)](https://github.com/r57zone/DSAdvance/blob/master/README.RU.md)

# DSAdvance
Продвинутая эмуляция Xbox геймпада для Sony DualSense, DualShock 4, Nintendo Pro контроллера или Joycon-ов. Поддерживается прицеливание и вождение наклонами геймпада, эмуляция стиков на тачпаде, эмуляция клавиатуры и мыши, а также [внешние педали](https://github.com/r57zone/XboxExternalPedals). Работает на базе драйвера [ViGEm](https://github.com/nefarius/ViGEmBus).

[![](https://user-images.githubusercontent.com/9499881/164945071-5b9f86dd-c396-45a5-817b-fc7068450f02.gif)](https://youtu.be/gkyqO_HuPnk)
[![](https://user-images.githubusercontent.com/9499881/164945073-cfa1bfb7-cb82-4714-b2ad-7ecd84a5bcfc.gif)](https://youtu.be/gkyqO_HuPnk)

# Режимы работы
Поддерживается несколько режимов работы, переключаются они нажатиями по тачпаду для DualSene и DualShock 4, а для Pro контроллеров, и JoyCon-ов на кнопки `Capture`, и `Home`.

![](https://user-images.githubusercontent.com/9499881/173076125-b3762211-74ab-4377-a6a2-a7b6c9b1a142.png)

Для выхода из режима эмуляции стиков на тачпаде нужно переключиться на режим по умолчанию. 


Нажимая на профиль по умолчанию, на DualSense, белые светодиоды отображают текущий статус заряда аккумулятора (1 - 0..25%, 2 - 26..50%, 3 - 51..75%, 4 - 76..100%), также на DualSense и DualShock 4 показывается статус батареи на световой панели (зелёный - 100..30%, жёлтый - 29..10%, красный - 9..1%), можно отключить в конфиге, параметр `ShowBatteryStatusOnLightBar`. Для DualSense и DualShock 4 отображается текущий заряд в самой программе.


Для изменения яркости 2 раза нажмите на область яркости. Если изменение яркости заблокировано, то подстветка будет выключаться по двойному клику.


Кнопка `PS` открывает "Xbox Game Bar" (нажимая `Win + G`), `PS + □` - уменьшить громкость, `PS + ○` - увеличить громкость, `PS + △` - увеличивает, а затем уменьшает чувствительность прицеливания (сброс на `PS + R3`), `PS + X` - кнопка микрофона (скриншот / нажатие сконфигурированной кнопки клавиатуры).


По умолчанию кнопка микрофона делает скриншот `Win + Alt + PrtScr` (для DualShock 4 нажимаем `PS + X`). Изменив параметр `MicCustomKey` на [нужное значение кнопки](https://github.com/r57zone/DSAdvance/blob/master/BINDINGS.RU.md) будет производится её нажатие.


Для эмуляции клавиатуры и мыши, для старых игр, переключите режим работы на `ALT + Q` или `PS + ←` и `PS + →` и выберите нужный профиль или [создайте новый профиль](https://github.com/r57zone/DSAdvance/blob/master/BINDINGS.RU.md). Профили переключаются на клавиши `ALT + ↑` и `ALT + ↓`, если окно активно или на геймпаде, с помощью `PS + ↑` и `PS + ↓`. Профиль по умолчанию позволяет работать в Windows. 


Для подключения внешних педалей измените номер COM-порта, изменив параметр `COMPort`, в разделе `ExternalPedals`.


Для выключения DualSense или DualShock 4 удерживайте кнопку PS в течении 10-15 секунд, пока контроллер не выключиться.

## Настройка
1. Установить [ViGEmBus](https://github.com/nefarius/ViGEmBus/releases).
2. Установить Microsoft Visual C++ Redistributable 2017 или новее.
3. Подключить геймпад Sony DualSense, DualShock 4, Nintendo Pro контроллер или JoyCon-ы.
4. Распаковать и запустить DSAdvance.
5. При необходимости изменить мёртвые зоны стиков, триггеров или другие параметры, в конфигурационном файле `Config.ini`.
6. При использовании со Steam играми, в настройках контроллера, отключите "персональные настройки Playstation".
7. Также рекомендуется установить [HidHide](https://github.com/nefarius/HidHide/releases), после чего в "HidHide Configuration Client" нужно добавить "DSAdvance.exe" и включить параметр `Enable device hiding` (если выключен). Необходимо для того, чтобы игра не видела наш контроллер, а видела только эмулируемый Xbox 360 геймпад.
8. (Необезательно) Для запуска из области уведомлений (tray), по двойному клику, можно добавить ярлык на `Launcher.exe` в автозагрузку Windows `%AppData%\Microsoft\Windows\Start Menu\Programs\Startup`.
9. (Необезательно) Для запуска сторонних утилит через Launcher укажатие имя и путь до приложения в конфигурационном файле.


## Возможные проблемы
• **Игра видит 2 геймпада одновременно (DualSense / DualShock 4 / Nintendo Pro контроллер или JoyCon-ы и Xbox)**<br>
Если игра поддерживает современный геймпад можно выключить эмуляцию Xbox геймпада на клавиши `ALT + Q` или вовсе скрыть этот геймпад, с помощью программы [HidHide](https://github.com/ViGEm/HidHide), или попробовать в беспроводном режиме.



• **Постоянное изменение значков клавиатуры и геймпада**<br>
Можно изменить режим прицеливания на "mouse-joystick", в программе или используйте прицеливание, с помощью левого триггера.



• **Не работают адаптивые триггеры или световая панель в игре**<br>
Добавьте игру в список исключений программы "HidHide" и изменить режим "DSAdvance" на "Only mouse".



• **Не работает вождение, в играх с поддержкой DualSense (без HidHide)**<br>
Сначала запустите DSAdvance, а только потом саму игру, игра может отдать приоритет эмулируемому геймпаду Xbox и вождение будет работать. Также можно включить режим эмуляции "Only driving & aiming", чтобы геймпад включался только в режиме вождения.



• **Не работает эмуляция клавиатура, в некоторых играх**<br>
В некоторых играх, например, Max Payne или Crysis 2, к сожалению, это пока не работает.

## Загрузка
>Версия для Windows 10, 11.

**[Загрузить](https://github.com/r57zone/DSAdvance/releases)**

## Благодарности
* Sony и Nintendo за самые продвинутые геймпады и инвестирование в инновации, а также за продвижение инноваций в игры.
* [ViGEm](https://github.com/ViGEm) за возможность эмуляции разных геймпадов.
* [HIDAPI library](https://github.com/signal11/hidapi), с [исправлениями](https://github.com/libusb/hidapi), за библиотеку для работы с USB устройства. В проекте используется этот [форк](https://github.com/r57zone/hidapi).
* [JoyShockLibrary](https://github.com/JibbSmart/JoyShockLibrary) за классную библиотеку геймпадов, позволяющую легко получить вращение контроллера. Также используется некоторый код из этой библиотеки.
* [Пользователям Reddit](https://www.reddit.com/r/gamedev/comments/jumvi5/dualsense_haptics_leds_and_more_hid_output_report/) за детальное описание выходного USB пакета DualSense.
* DS4Windows[[1]](https://github.com/Jays2Kings/DS4Windows)[[2]](https://github.com/Ryochan7/DS4Windows) за уровень заряда батареи.
* ChatGPT за улучшения прицеливания.

## Сборка
1. Загрузите исходники и распакуйте.
2. [Загрузите Microsoft Visual Studio Code 2017+](https://code.visualstudio.com/download) и [установите](https://github.com/r57zone/RE4ExtendedControl/assets/9499881/69dafce6-fd57-4768-83eb-c1bb69901f07).
3. Измените в свойствах проекта набор инструментов и SDK на ваш.
4. Выберите тип сборки `Release` (если установлен `Debug`) и `x86`, после чего скомплириуйте проект.

## Обратная связь
`r57zone[собака]gmail.com`