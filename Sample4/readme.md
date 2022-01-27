# Модель над-транспортного уровня Transport+

# Хорда

Содержание

[TOC]

## Used libraries

В проекте **модели Над-транспортного уровня** (далее - модели Хорды) используется **STL**, библиотека **SystemC** **2.2.3** для параллельного моделирования и библиотека **Nlohmann** для работы с JSON файлами (**config.txt** и **./log/snapshot.txt**).

## Limitation use

Для работы с большой размерностью ID идентификаторов узлов (SHA-1, 160bit) используется шаблонный класс sc_biguint<w> из SystemC. На этапе компиляции (до run time) требуется определить размерность адресного пространства ` w`. В связи с этим, для моделирования разных адресных пространств следует перекомпилировать проект модели Хорды для разных величин `w`. Размерность адресного пространства в модели задается в файле `inc.h` в объявлении `typedef ... uint160` и `typedef ... uint168`.

## How to compile

1. **Скомпилировать библиотеку** SystemC.

   Инструкция для компиляции под Linux и Windows представлена в файле `Sample4\systemc-2.3.3\INSTALL`. Файл `INSTALL` открыть любым текстовым редактором (блокнот). .
   
   Краткое изложение инструкции `INSTALL`:
   
   - Перейти в папку `.\Sample4\systemc-2.3.3\msvc10\SystemC` и открыть файл проекта `SystemC.sln`  в Visual Studio.
   - Выполнить компиляцию `SystemC` и `SystemC-core` в режиме `Debug x64` и `Release x64`. Режим компиляции задается в верхней строке Visual Studio.
   - Будет скомпилирован systemc.lib, который будет помещен в `.\Sample4\systemc-2.3.3\msvc10\SystemC\x64\Debug` и `.\Sample4\systemc-2.3.3\msvc10\SystemC\x64\Release`
   
   

  **ВАЖНО!**

- Задать одинаковое значение параметра компиляции `Runtime Library`

  При компиляции под Windows - в настройках проекта `Configuration Properties -> С\С++ -> Code Generation -> Runtime Library` обоих проектов `sample4.vcxproj` и `SystemC.vcxproj`  должно стоять одинаковое значение. Например, для режима `Debug x64` можно задать:

  ```
  Configuration Properties -> С\С++ -> Code Generation -> Runtime Library =  Multi-threaded Debug DLL (/MDd)
  ```

  Для режима `Release x64` задать `Multi-threaded DLL (/MD)`.

  Библиотека SystemC должна быть скомпилирована с тем же параметром, что и sample4. Иначе, будут ошибки во время линковки.

  

- Убрать сообщение `"Unknown compiler version - please run ..."`

  Следует открыть файл:

  `.\systemc-2.3.3\src\sysc\packages\boost\config\compiler\visualc.hpp`

  и заменить строку 175

  ```c++
  175   #if (_MSC_VER > 1500) 
  ```

  на строку с более поздней версией Вашего компилятора

  ```c++
  175   #if (_MSC_VER > 1910)
  ```

  

2. **Скомпилировать проект** `sample4`. 

   Под Windows - открыть с помощью Visual Studio 2019 (или выше версии) файл проекта `sample4.vcxproj`. 

   Использовать инструкцию `INSTALL`, поставляемую вместе с библиотекой SystemC. Действия из `INSTALL` в кратком виде приведены ниже:

   - Задать `Enable Run-Time Type Info` значение `Yes` для параметра `Language properties` во вкладке настроек `Configuration Properties -> С/С++` .

   - В окне `Additional Options` задать `/vmg` для параметра `Command Line` во вкладке настроек `Configuration Properties -> С/С++` .

   - Для параметра `Additional Dependencies` задать `systemc.lib` во вкладке настроек `Configuration Properties -> Linker-> Input`, прописать путь, где лежит systemc.lib.

   - В настройках проекта установить параметр `Additional Include Directorie` на папку с SystemC:

   ```c++
   Configuration Properties -> C/C++ -> General -> Additional Include Directories = .\systemc-2.3.3\src
   ```

   ​	Также следует прописать путь до исходников Хорды, добавив строку `./src`:

   ```c++
   Configuration Properties -> C/C++ -> General -> Additional Include Directories = .\systemc-2.3.3\src;.\src
   ```

   ​	И добавить путь до исходников библиотеки Nlohmann:

   ```c++
   Configuration Properties -> C/C++ -> General -> Additional Include Directories = .\systemc-2.3.3\src;.\src;.\json-release-3.10.4\include;
   ```

   **ВАЖНО!**

   Эти действия следует выполнить для `Debug x64` и `Release x64`.

   

## Address bit range

Для задания адресного пространства ID узлов следует открыть файл `./src/inc.h` и внести изменения в строки объявления `uint160` и `uint168`:

- для адресов 4 bit, диапазон от 0 до 16:

  ```c++
  25    typedef sc_biguint<4>      uint160;
  26    typedef sc_biguint<4+8>    uint168;
  ```

  для адресов 10 bit, диапазон от 0 до 1023:

- ```c++
  25    typedef sc_biguint<10>      uint160;
  26    typedef sc_biguint<10+8>    uint168;
  ```

  для адресов 20 bit, диапазон от 0 до 1048575:

- ```C++
  25    typedef sc_biguint<20>      uint160;
  26    typedef sc_biguint<20+8>    uint168;
  ```

  

## Input parameters

### config.txt - основные параметры

Файл `config.txt` используется для запуска модели Хорды и содержит основные входные параметры для запуска модели. Файл задается и хранится в JSON формате. Файл всегда прочитывается моделью. Если файл содержит недопустимые символы и нарушены правила JSON формата, то будет выдано предупреждение пользователю.

В `config.txt` задаются следующие параметры:

- `"arrayID"` - содержит в ручную задаваемый набор ID узлов. Первым в модели будет запущен узел, чей ID расположен в начале списка (левый край), затем следующий и так далее, до конца списка.
- `"gen_random"` - флаг генерации (псевдо)случайных ID для узлов. Если флаг `true`, то будет сгенерированы ID в количестве, равном `"nodes"`, а параметр `"arrayID"` будет проигнорирован. Не может быть сгенерировано ID узлов больше текущей размерности адресов. Если используется 4 bit размерность, то максимум может быть сгенерировано 16 узлов от 0 до 15.
- `"nodes"` - задает количество идентификаторов узлов при (псевдо)случайной генерации. Выполняется, `"gen_random": true`.
- `"shuffle"` - перемешивание (псевдо)случайным образом идентификаторов узлов, заданных в `"arrayID"`. Выполняется, если `"gen_random": false`.
- `"snapshot_period_sec"` - период (в секундах модельного времени) вывода снепшотов fingers со всех работающих узлов на жесткий диск. При `"snapshot_period_sec": 1.0`  cнэпшот будет выводиться каждую 1 секунду модельного времени.  Снепшот помещается в файл `.log/snapshot.txt`. Рекомендуется задать `"snapshot_period_sec": 999999999.0`, снэпшот будет выведен после моделирования.

### Таймеры, счетчики, размеры очередей - внутренние параметры

Таймауты (величина таймера), счетчики, размеры очередей задаются в следующих файлах исходного кода:

- **main.cpp**:

  - `DELAY_TURN_ON` - период включения узлов, секунды. 1-й узел был запущен в 0 момент времени,  2-й узел через DELAY_TURN_ON,  следующий - вновь через DELAY_TURN_ON и так далее.
  - `ADD_SIM_TIME` - дополнительное время, которое дается модели после включения последнего узла, чтобы все сообщения были доставлены и fingers всех узлов стабилизировались

  - `SIM_TIME`  - суммарное общее время моделирования, на которое запускается вся модель сети, вычисляется автоматически от количества узлов, `DELAY_TURN_ON` и `ADD_SIM_TIME`.
  - `FINGERS_SIZE` - количество fingers, вычисляется автоматически от размерности адресного пространства (при 4bit будет 4 fingers).

- ./src/trp/**llchord_classes_defs.h**:

  - `DEFAULT_TIMEOUT_UPDATE` - период проверки fingers, successor, predecessor (период запуска fix_fingers, stabilize)

  - `DEFAULT_TIMEOUT_RX_SUCCESSOR_ON_JOIN` - таймаут ожидания прихода сообщения successor в ответ на отправленное сообщение join

  - `DEFAULT_TIMEOUT_RX_SUCCESSOR` - таймаут ожидания прихода сообщения successor в ответ на отправленное сообщение find_successor

  - `DEFAULT_TIMEOUT_RX_PREDECESSOR` - таймаут ожидания прихода сообщения predecessor в ответ на отправленное сообщение find_predecessor

  - `DEFAULT_TIMEOUT_RX_ACK` - таймаут ожидания прихода сообщения подтверждения ack

    

## Output parameters

Таймеры включения узлов задаются в main.cpp:

- snapshot.txt 
- app.txt
- trp1.txt ... trp2000.txt



### snapshot.txt

Cодержит снимок снэпшот состояния fingers каждого работающего узла в конкретный модельный момент времени.

Пример снэпшота для трех узлов приведен ниже:

...



## WARNING MESSAGES

В ходе работы модели могут быть выданы следующие сообщения:

- Если список ID идентификаторов узлов `arrayID` в `config.txt` содержит повторяющиеся числа, то SystemC автоматически переименует их в собственный формат, моделирование будет **не корректным**.

  ```c++
  Warning: (W505) object already exists: ... Latter declaration will be renamed to ...
  ```

  Исключите из arrayID повторяющиеся числа. Повторно запустите модель.
