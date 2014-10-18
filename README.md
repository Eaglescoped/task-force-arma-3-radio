Task Force Arma 3 radio
=======================
#Поддержите рацию на конкурсе [MakeArmaNotWar](http://makearmanotwar.com/entry/pMP8c7vSS4#.VA1em_nV9UD)!

Arma 3 Team Speak Radio Plugin ([документация](https://github.com/michail-nikolaev/task-force-arma-3-radio/wiki))
_v0.9.5 от 16.10.2014_

#####Совместима с 0.9.3 и 0.9.4, не совместима с 0.9.2

###Установка

* Скачайте [архив с рацией 0.9.5](https://github.com/michail-nikolaev/task-force-arma-3-radio/raw/master/releases/0.9.5.zip) и распакуйте его.
* Cкопируйте содержимое папки `TeamSpeak 3 Client` в корневую папку TeamSpeak.
* Скопируйте содержимое папки `Arma 3` в папку с игрой `...\SteamApps\common\Arma 3`.

> TFAR использует последнюю версию СBA (Community Base Addons). Поэтому, если этот мод у вас уже установлен, Windows предложит заменить папку.


###Настройка

* Убедитесь, что в TeamSpeak клавиша `Caps Lock` не используется для разговора.
* Также в ARMA 3 отключите разговор по внутренней связи (VON) по нажатию `Caps Lock` (чтобы не двоиться).
* Откройте список плагинов в Team Speak `Settings > Plugins`.
  1. Включите `Task Force Arma 3 Radio`.
  2. Отключите `ACRE` и `radio ts ARMA3.ru version`, если они есть, чтобы избежать конфликтов.
  3. На всякий случай внизу слева можно нажать кнопку Reload All для перезагрузки всех плагинов.
* Убедитесь, что громкость оповещений в Team Speak не отключена: `Options > Payback > Sound Pack Volume` установите в положительное значение.
* Запустите игру с аддонами `@CBA_A3` и `@task_force_radio` (Community Base Addons: A3 Beta и Task Force Arrowhead Radio). Это можно сделать, прописав в ярлыке игры имена модов после EXE файла `…\arma3.exe -mod=@CBA_A3;@task_force_radio`, но рекомендуется включить необходимые моды в настройках игры (Настройки - Дополнения, Settings -> Expansions).
* Зайдите в тот же канал, где находятся другие игроки, играющие с данной рацией, либо вас перебросит в канал `TaskForceRadio` в случае его наличия при старте миссии.

> Если ник в профиле игры и TeamSpeak совпадает - плагин изменит ваш ник в TS в процессе игры.

> Не рекомендуется использовать плагин при одновременном подключении к нескольким серверам в TeamSpeak

> Рекомендуется отключить встроенные звуки оповещений TeamSpeak: `Options > Notifications > Sound Pack: "Sounds Deactivated"`. Для применения этой опции необходимо перезапустить TeamSpeak.


###Использование

| Клавиши | Действие |
| --- | --- |
| Кнопка разговора в TeamSpeak | Прямая речь. |
| `Caps Lock` | Разговор по рации. |
| `CTRL`&nbsp;+&nbsp;`Caps Lock` | Разговор по рации дальней связи. |
| `CTRL`&nbsp;+&nbsp;`P` | Открыть интерфейс личной рации (рация должна быть в слоте инвентаря). В том случае, если у вас имеются несколько раций - вы сможете выбрать требуемую. Также есть возможность установить рацию как активную (ту, которая будет использоваться для передачи). Кроме того, существует возможность скопировать настройки для раций и другой радиостанции, если она использует аналогичный код шифрования.
| `NUM[1-8]` | Быстрое переключение каналов коротковолновой рации. | 
| `ALT`&nbsp;+&nbsp;`P` | Открыть интерфейс рации дальней связи (рация дальней связи должна быть надета на спину, либо вы должны быть в технике за водителя, стрелка, командира или помощника пилота). Если доступно несколько раций - вам будет предложено выбрать. Также одну из них можно установить как активную. Кроме того, существует возможность скопировать настройки для раций и другой радиостанции, если она использует аналогичный код шифрования. |
| `CTRL`&nbsp;+&nbsp;`NUM[1-9]` | Быстрое переключение каналов рации дальней связи. |
| `CTRL`&nbsp;+&nbsp;`TAB` | Изменить громкость прямой речи. Можно говорить шепотом (Whispering), нормально (Normal) и кричать (Yelling). Не влияет на громкость сигнала в радио передаче. |
| `SHIFT`&nbsp;+&nbsp;`P` | Открыть интерфейс переговорного устройства для дайверов (на вас должен быть надет ребризер).| 
| `CTRL`&nbsp;+&nbsp;`~` | Разговор по переговорному устройству для дайверов. |
| `CTRL`&nbsp;+&nbsp;`]`| Следующая рация ближней связи. |
| `CTRL`&nbsp;+&nbsp;`[`| Предыдущая рация ближней связи. |
| `CTRL`&nbsp;+&nbsp;`ALT`&nbsp;+&nbsp;`]`| Следующая рация дальней связи. |
| `CTRL`&nbsp;+&nbsp;`ALT`&nbsp;+&nbsp;`[`| Предыдущая рация дальней связи. |
| `CTRL`&nbsp;+&nbsp;`[←,↑,→]`| Изменение стерео режима рации ближней связи. |
| `ALT`&nbsp;+&nbsp;`[←,↑,→]`| Изменение стерео режима рации дальней связи. |
| `T` | Передача на дополнительном канале персональной рации. |
| `Y` | Передача на дополнительном канале рации дальней связи. |
| `ESC` | Выход из интерфейса рации. |

###Информация

#####Рации

| Рация | Сторона | Диапазон/Дальность | 
| --- | --- | --- | --- | 
| Рация [AN/PRC-152](http://ru.wikipedia.org/wiki/AN/PRC-152) (персональная рация командира) | <font color="blue">BLUEFOR<font> | 30-512Mhz / 5 км |
| Рация [RF-7800S-TR](http://rf.harris.com/capabilities/tactical-radios-networking/rf-7800s-tr.asp) (личная рация бойца) | <font color="blue">BLUEFOR<font> | 30-512Mhz / 2 км |
| Рация [RT-1523G (ASIP)](http://en.wikipedia.org/wiki/SINCGARS#Models) (дальняя связь) | <font color="blue">BLUEFOR<font> | 30-87Mhz / 20 км (30 для встроенной) |
| Рация [AN/ARC-210)](http://www.rockwellcollins.com/~/media/Files/Unsecure/Products/Product%20Brochures/Communcation%20and%20Networks/Communication%20Radios/ARC-210%20Integrated%20Comm%20Systems%20white%20paper.aspx) (авиационная) | <font color="blue">BLUEFOR<font> | 30-87Mhz / 40 км |
| Рация [AN/PRC148-JEM](https://www.thalescomminc.com/ground/anprc148-jem.asp) (персональная рация командира) | <font color="green">INDEPENDENT</font> | 30-512Mhz / 5 км | 
| Рация [AN/PRC-154](http://www.gdc4s.com/anprc-154a-rifleman-radio.html) (личная рация бойца) | <font color="blue">INDEPENDENT<font> | 30-512Mhz / 2 км | 
| Рация [AN/PRC-155](http://www.gdc4s.com/anprc-155-2-channel-manpack.html) (дальняя связь)| <font color="green">INDEPENDENT</font>  | 30-87Mhz / 20 км (30 для встроенной) | 
| Рация [AN/ARC-164](https://ru.wikipedia.org/wiki/AN/ARC-164) (авиационная) |  <font color="green">INDEPENDENT</font> | 30-87Mhz / 40 км |
| Рация [FADAK](http://www.iran.ru/news/politics/87228/Iran_prodemonstriroval_tri_novyh_obrazca_voennogo_naznacheniya) (персональная рация командира) |  <font color="red">OPFOR</font> | 30-512Mhz / 5 км | 
| Рация [PNR-1000A](http://elbitsystems.com/Elbitmain/files/Tadiran%2520PNR1000A_2012.pdf) (личная рация бойца) | <font color="blue">OPFOR<font> | 30-512Mhz / 2 км | 
| Рация [MR3000](http://www.railce.com/cw/casc/rohde/m3tr.htm) (дальняя связь) | <font color="red">OPFOR</font> | 30-87Mhz / 20 км (30 для встроенной) | 
| Рация [MR6000L](http://www.rohde-schwarz.com/en/product/mr6000l-productstartpage_63493-9143.html) (авиационная) | <font color="red">OPFOR</font> | 30-87Mhz / 40 км | 
| Переговорное устройство дайверов | Все | 32-41kHz / 70-300 м. (в зависимости от уровня волн) |

> В списке перечислены не все рации, дополнительно представлены следующие авиационные радиостанции (дальность - 40 км): "AN/ARC-201", "AN-ARC-164", "MR6000L"

> Личная и дальняя рации одной фракции поддерживают единый протокол, поэтому могут связываться друг с другом. В случае если передача осуществляется с личной -  звук будет высокочастотным. В случае передачи с дальней - низкочастотным. 

> Распространению радио сигнала мешает ландшафт. Худший случай - вы прямо за крутым холмом. Если вы начнете двигаться от края холма в направлении от передающего - сигнал будет улучшаться. Лучший случай - прямая видимость.

> Персональная рация командира и рация дальней связи позволяют одновременно принимать и передавать два канала. Нажатие на рации "Настроить дополнительный канал" сделает текущий канал дополнительным. Переключившись на другой канал вы будет слышать два канала - активный и дополнительный. Возможно настроить различные стерео режимы для активного и дополнительного каналов. Используя клавиши `T` и `Y` можно осуществлять передачу в дополнительный канал персональной и дальней раций соответственно.

#####Выдача раций
* По умолчанию рация дальней связи выдается лидерам отрядов. Если у игрока надет рюкзак - он его автоматически положит на землю.

* Рация ближней связи выдается игрокам, у которых есть `ItemRadio` в инвентаре. Выдача рации может потребовать нескольких секунд ожидания (следите за сообщениями в центре экрана).

#####Техника
* Рация дальней доступна в технике водителю, командиру, стрелку и помощнику пилота. Не вся техника имеет встроенные рации.

* У каждого слота техники своя рация, которая должна быть настроена отдельно. Если вы планируете пересаживаться с места на места в технике - предварительно настройте рацию на всех слотах (например на слоте водителя и на слоте стрелка).

* Техника делиться  на открытую и закрытую (изолированную). Если вы находитесь в изолированной технике, то почти не будете слышать голоса снаружи (и наоборот). Однако если вы выгляните из техники, то будете слышать как голоса внутри, так и снаружи.

#####Радиоперехват

* Рации можно поднимать у убитых игроков, передавать их друг другу. При этом они сохраняют все настройки (каналы, частоты, громкость). 

> Рекомендуется поднимать рации открывая инвентарь на том месте, где она лежит (чтобы она не пропала из-за ошибок игры). 

* Настройки рации в технике также сохраняются.
* По умолчанию рации каждой фракции используют свои коды шифрования, поэтому вы не будете слышать вражеские переговоровы, даже введя используемую врагами частоту. Чтобы прослушивать (и говорить в эфир) противника - необходимо каким-либо образом захватить вражескую радиостанцию.

> Для того, чтобы прослушивать рацию дальней связи (рюкзак) противника рекомендуется находиться в своей технике. В таком случае вы сможете слушать эфир противника использую рюкзак, и передавать союзникам использую рацию в технике, как активную.

#####Водолазы
* Вы не можете говорить голосом находясь под водой (даже в водолазном костюме). Однако на близком расстоянии ваш собеседник сможет расслышать что-либо очень невнятное (исключение - если вы под водой в изолированной технике).
* Находясь под водой, вы невнятно и слабо слышите голоса на суше.
* Для связи между дайверами используйте Переговорное устройство.
* Вы не можете пользоваться радио связью под водой (ни говорить, ни слышать). Если нужно что-то передать на сушу - всплывайте. Исключение - субмарина на перископной глубине (в ней водолазам можно использовать рацию дальней связи).

#####Режимы работы плагина
Плагин поддерживает два режима работы — **серьезный** и **упрощенный**. 

* **Упрощенный** — используется по умолчанию. Он предназначен главным образом для кооперативных игр. Его особенностью является то, что игроки с плагином и в игре слышат мертвых, не играющих, играющих на другом сервере и играющих без плагина игроков минуя рацию (просто как через TeamSpeak). Это делает менее удобным игры против людей, но позволяет вашему другу без проблем узнать, где вы играете, какая у вас частота и т.д. Разумется, те кто играет на одном сервере со включенными аддонами и плагинами будут слышать друг друга по "законам" рации с учетом расстояния и частот.

* **Серьезный** — предназначен для проведения игр, где игроки играют против других игроков. Для его активации в TeamSpeak необходимо создать канал с названием `TaskForceRadio` и паролем `123`. Игроки должны включить плагин рации, зайти на сервер и, разойдясь по каналам сторон, брифинговать. При старте миссии через несколько секунд игроки будут переброшены в канал `TaskForceRadio`. В данном случае игроки будут слышать только живых игроков со включенным плагином играющих на этом же сервере. Мертвые игроки, в свою очередь, могут общаться друг с другом. Если мертвый игрок респавниться - он снова будет слышать только живых. После того, как игра заканчивается, игроки перебрасываются в канал, в котором они брифинговали до игры.

#####Решение проблем
* `Pipe error 230` - вы скорее всего вы забыли включить плагин в TeamSpeak.
* Плагин в TS показываться красным и не загружается - скорее всего вам нужно обновить TeamSpeak.
* Если что-то поломалось - попробуйте перезапустить плагин.
* Не работают действия на `Caps Lock` - возможно из-за геймерской клавиатуры, где код `Caps Lock` отличается. Попробуйте изменить используемые клавиши (путем редактирования `userconfig`).
* Если из-за ошибки или еще чего-то вы перестали слышать других игроков даже вне игры, откройте `Setup 3D Sound` в TeamSpeak и кликните `Center All`.
* Для устранения возможных ошибок с плагином разработчикам может потребоваться лог TeamSpeak, чтобы его скопировать выберите `Tools` -> `Client Log`, поставьте все галочки сверху и, выделив весь текст на `CTRL + A` Скопируйте его в буфер обмена
* Если TeamSpeak (тьфу-тьфу-тьфу) упал при использовании плагина - он показывает окошко с описанием того, где можно найти дамп (путь к файлу). Буду очень благодарен за этот файлик. 

#####Администраторам TeamSpeak серверов
На всякий случай уменьшите уровень защиты от флуда: `Правый клик по серверу > Edit Virtual Server > More > Anti Flood` поставьте значения 30, 300, 3000 (сверху вниз).


#####Разработчикам
Если данная разработка будет как-либо популярна, то хотелось бы избежать кучи несовместимых форков. По этой причине в случае желания внесения изменений связывайтесь со мной - велика вероятность, что ваши разработки будут вмержены в главную ветку. Ждем ваших Pull Requests :)

#####Спасибки
* Отряду [Task Force Arrowhead](http://forum.task-force.ru/) за тестирование, поддержку, терпение и всяческую помощь.
* [MTF](http://forum.task-force.ru/index.php?action=profile;u=7) ([varzin](https://github.com/varzin)) за помощь с графикой и документацией.
* [Hardckor](http://forum.task-force.ru/index.php?action=profile;u=14) за помощь с графикой.
* [Shogun](http://forum.task-force.ru/index.php?action=profile;u=13) за помощь с графикой.
* [Блендеру](http://arma3.ru/forums/index.php/user/41-blender/) за шрифт.
* [vinniefalco](https://github.com/vinniefalco) за [DSP фильтры](https://github.com/vinniefalco/DSPFilters).
* [WOG](http://wogames.info/) и лично [TRUE](http://wogames.info/profile/TRUE/) за помощь в тестировании.
* [Music DSP Collection](https://github.com/music-dsp-collection) за компрессор.
* [Avi](http://arma3.ru/forums/index.php/user/715-avi/) за кодревью.
* [andrey-zakharov](https://github.com/andrey-zakharov) ([Vaulter](http://arma3.ru/forums/index.php/user/1328-vaulter/)) за помощь в разработке.
* Дине за перевод.
* [Zealot](http://forums.bistudio.com/member.php?125460-zealot111) за помощь в разработке и полезные скрипты.
* [NouberNou](http://forums.bistudio.com/member.php?56560-NouberNou) за советы и конкуренцию.
* [Megagoth1702](http://forums.unitedoperations.net/index.php/user/2271-megagoth1702/) за свою давнюю работу по эмуляции звучания рации.
* [Naught](http://forums.unitedoperations.net/index.php/user/6555-naught/) за ревью.
* [Andy230](http://forums.bistudio.com/member.php?100692-Andy230) за перевод.* [L-H](http://forums.bistudio.com/member.php?87524-LordHeart) for code changes.
* [NorX_Aengell](http://forums.bistudio.com/member.php?99450-NorX_Aengell) for French translation.
* [lukrop](http://forums.bistudio.com/member.php?78022-lukrop) за помощь с кодом.
* [nikolauska](http://forums.bistudio.com/member.php?75014-nikolauska) ([GitHub](https://github.com/nikolauska)) за помощь с SQF кодом.
* [Kavinsky](https://github.com/kavinsky) за AN/PRC-154, RF-7800S-TR и другие рации.
* [JonBons](http://forums.bistudio.com/member.php?81374-JonBons) за помощь с кодом.
* [ramius86](https://github.com/ramius86) за итальянский перевод.
* KK за [статьи](http://killzonekid.com/arma-scripting-tutorials-float-to-string-position-to-string/)
* [Krypto202](http://www.armaholic.com/users.php?m=details&id=45906&u=kripto202) за звуки.
* [pastor399](http://forums.bistudio.com/member.php?128853-pastor399) за модель рюкзаков и текстуры.
* [J0nes](http://forums.bistudio.com/member.php?96513-J0nes) за помощь с моделями.
* [Raspu86](http://forums.bistudio.com/member.php?132083-Raspu86) за модель рюкзаков.
* [Gandi](http://forums.bistudio.com/member.php?111588-Gandi) за текстуры.
* [Pixinger] (https://github.com/Pixinger) за помощь с Зевсом.
* [whoozle] (https://github.com/whoozle) за звуковой движок и помощь.
* [CptDavo](http://forums.bistudio.com/member.php?75211-CptDavo) за помощь с текстурами.
* [KoffeinFlummi](https://github.com/KoffeinFlummi) за помощь с кодом.
* [R.m.K Gandi](http://steamcommunity.com/profiles/76561197984744647/) за текстуры рюкзаков.
* [Pomigit](http://forums.bistudio.com/member.php?97133-pomigit) за паттерны текстур.
* [Priestylive](https://plus.google.com/u/0/113553519889377947218/posts) за текстуры для BWMOD.
* Всем, кто делал видео и статьи с обзорами.
* Всем пользователям (особенно тем, что нашли баги).
* Извините, если кого-то случайно забыл.