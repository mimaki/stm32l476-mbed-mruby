# mruby on NUCLEO-L476RG (stm32l476-mbed-mruby)

STMicroelectoronics製のマイコン評価ボード[NUCLEO-L476RG](https://www.st.com/content/st_com/ja/products/evaluation-tools/product-evaluation-tools/mcu-eval-tools/stm32-mcu-eval-tools/stm32-mcu-nucleo/nucleo-l476rg.html)に[mruby](https://github.com/mruby/mruby)をポーティングしました。

mrubyは、人気の開発言語「[Ruby](https://www.ruby-lang.org)」を軽量化したプログラミング言語で、組込みシステムや様々なソフトウェアに組み込むことができる高機能なプログラミング言語です。

[NUCLEO-L476RG](https://www.st.com/content/st_com/ja/products/evaluation-tools/product-evaluation-tools/mcu-eval-tools/stm32-mcu-eval-tools/stm32-mcu-nucleo/nucleo-l476rg.html)は、ARM Cortex-M4 80MHz, 1MB FLASH, 128KB SRAMを搭載したマイコン評価ボードで、mbed OSにも対応しています。

(カスタマイズなしの)mrubyは200KB程度のRAMを必要としますが、stm32l476-mbed-mrubyでは、下記を行うことで128KB(実際は16KB+96KB)のSRAMでmruby-1.3.0[^1]を動作させています。

- mrubyを省メモリ向けにカスタマイズ
- メモリ管理を自作

マイコンボードはNUCLEO-L476RGを使用していますが、SRAMが128KB以上、FLASHが256KB以上あるマイコンであればmrubyを動作させることは可能です。

128KBのRAMで本格的なアプリケーションを動作させることは流石に難しいですが、安価で入手可能なマイコンボードでも動作する軽量版Ruby「mruby」を是非体験してみて下さい。

[^1]: 公開時点のmrubyの最新バージョンは1.4.1ですが、1.3.0の時代にポーティングを実施したため、少し古いmrubyを使用しています。


# 導入手順

## 1. 準備するもの

- NUCLEO-L476RG マイコンボード
- Windows PC / Mac
- USB A-miniB ケーブル


----

## 2. ビルド環境の構築

### 2.1. C言語開発環境のインストール

#### Windowsの場合

Windows環境でのmrubyのビルドには、Microsoft Visual Studio(VC++)、MinGW(gcc)、Cygwin(gcc)等が利用可能ですが、ここでは[MinGW](http://www.mingw.org/)を使用する前提で説明していきます。  
[MinGWのダウンロードサイト](https://sourceforge.net/projects/mingw/files/Installer/)から**mingw-get-setup.exe**をダウンロードして実行します。（インストール時の設定はデフォルトのままでOKです）

**MinGW Installation Manager**の画面が表示されたら
- mingw32-base
- mingw32-gcc-g++
- msys-base

の3項目を選択して、**Installation**メニューの**Apply Changes**を実行して下さい。  
インストール完了後、以下のファイルのショートカットをデスクトップに作成します。
```
C:¥MinGW¥msys¥1.0¥msys.bat
```
**注意**  
以下、Windows環境でのコマンド実行は、このmsys.batを実行して表示されるコマンド画面から実行するものとします。


#### Macの場合  
XcodeのCommand Line Toolsをインストールして下さい。


### 2.2. Rubyのインストール

mrubyをビルドするためには、本家Rubyが必要です。  

#### Windowsの場合  
[RubyInstaller](https://rubyinstaller.org/)より、2.0.0以降のバージョンのインストーラをダウンロードして、Rubyをインストールして下さい。  

#### Macの場合  
プリインストールされているRuby、またはrbenvなどでインストールされたRubyがそのまま利用できます。


### 2.3. GNU Bisonのインストール

#### Windowsの場合  
[Bison for Windows](http://gnuwin32.sourceforge.net/packages/bison.htm)より、**Complete package, except sources**の**Setup**をダウンロードして、Bisonをインストールして下さい。  
インストール先のフォルダ名は **C:¥GnuWin32** に指定するものとします。  
インストール実行後、環境変数 **PATH** に以下を追加して下さい。
```
C:¥GnuWin32¥bin
```

#### Macの場合  
**Homebrew**を使用して、以下のコマンドでBisonをインストールします。
```
$ brew install bison
```

以下のコマンドを実行し、上記それぞれにパスが通っていることを確認します。


### 2.4. mbed CLIのセットアップ

**momo-mruby** では mbed (GR-PEACH) 用にクロスコンパイルするために [mbed CLI](https://github.com/ARMmbed/mbed-cli) を使用します。  
mbed CLIのセットアップ手順を以下に示します。

#### 2.4.1. Python 2.7  
mbed CLI を利用するためには Python 2.7 が必要です。(Python 3は利用できません)  
[Python 2.7](https://www.python.org/downloads/release/python-2712/) をセットアップしてください。  
※ momo-mruby上ではPythonは動作しません。

#### 2.4.2. Git  
[Git](https://git-scm.com/) 1.9.5 以降をインストールしてください。

#### 2.4.3. Mercurial  
[Mercurial](https://www.mercurial-scm.org/) 2.2.2 以降をインストールして下さい。

#### 2.4.4. GNU ARM Embedded Toolchain  
[GNU ARM Embedded Toolchain 5.4](https://launchpad.net/gcc-arm-embedded/5.0/5-2016-q2-update) をインストールしてください。

以下のコマンドを実行し、上記それぞれにパスが通っていることを確認します。

**Python**
```
$ python --version
Python 2.7.xx
```
**Git**
```
$ git --version
git version 2.x.xxxxxx
```
**Mercurial**
```
$ hg --version
Mercurial - 分散構成管理ツール(バージョン 4.1.2)
(詳細は https://mercurial-scm.org を参照)

Copyright (C) 2005-2017 Matt Mackall and others
本製品はフリーソフトウェアです。
頒布条件に関しては同梱されるライセンス条項をお読みください。
市場適合性や特定用途への可否を含め、 本製品は無保証です。
```
**GNU ARM Toolchain**
```
$ arm-none-eabi-gcc --version
arm-none-eabi-gcc (GNU Tools for ARM Embedded Processors) 5.4.1 20160609 
...
```

コマンド実行がうまくいかない（パスが通っていない）場合は、パスを追加して下さい。

##### Windowsの場合  
システムのプロパティ - 詳細設定 - 環境変数から、環境変数 **PATH** に以下を追加します。
- C:¥Python27
- C:¥Python27¥Scripts
- C:¥Program Files¥Git¥cmd
- C:¥Program Files (x86)¥GNU Tools ARM Embedded¥5.4 2016q2¥bin  
※ 32bit版Windowsの場合は C:¥Program Files¥GNU Tools ARM Embedded¥5.4 2016q2¥bin

##### Macの場合  
```
$ export PATH=$PATH:$INSTALL_DIR/gcc-arm-none-eabi-5_4-2016q2/bin
```

※ **$INSTALL_DIR**には、GNU ARM Toolchainsをインストールしたディレクトリを指定して下さい。


#### 2.4.5. mbed CLI  
上記1〜4のインストールが完了したら、mbed CLIをインストールします。
```
$ pip install mbed-cli
```


----

## 3. ソースコードの取得とビルド

以下のコマンドを実行して、 **stm32l476-mbed-mruby** のソースコードを取得します。

```
$ cd $WORKING_DIR
$ git clone https://github.com/mimaki/stm32l476-mbed-mruby.git --recursive
$ cd stm32l476-mbed-mruby
$ make first
```

* **$WORKIND_DIR**には、任意のディレクトリを指定して下さい。
* ```make first```は初回のみ実行して下さい。

### ソースコードのビルド

```
$ make
```

---

## 4. マイコンボードへの書き込み

NUCLEO-L476RGをUSBケーブルでPCに接続すると ```NODE_L476RG``` ドライブとして認識されます。

以下のファイルを ```NODE_L476RG``` ドライブのルートディレクトリにコピーすることで、```NUCLEO-L476RG``` のファームウェアが更新・再起動されます。

```
BUILD/NUCLEO_L476RG/GCC_ARM/stm32l476-mbed-mruby.bin
```


## 5. 動作確認

サンプルアプリケーション ([app.rb](https://github.com/mimaki/stm32l476-mbed-mruby/blob/master/app.rb)) では、putsによるコンソール出力とDigiralIO#writeによるLED制御が行われます。

コンソール出力を確認するためには、VCP(Virtual COM Port)ドライバとターミナルソフトをインストールする必要があります。

### 5.1. VCPドライバのインストール

#### Windowsの場合

STmicroelectronicsの[ダウンロードサイト](https://www.st.com/ja/development-tools/stsw-stm32102.html)からVCPドライバをダウンロード・インストールして下さい。

#### Macの場合

FTDIが提供している[VCP Driver](http://www.ftdichip.com/Drivers/VCP.htm)をダウンロード・インストールして下さい。


### 5.2. ターミナルソフトのインストール

putsによるコンソール出力を確認するためには、CoolTermなどのターミナルソフトを使用します。
ターミナルソフトで下記を設定し、NUCLEO-L476RGをUSB接続してCOMポートに接続すると、コンソール出力が確認できます。

|項目|値|
|:-:|:--|
|Port|Windowsの場合: COMx<br />Macの場合: usbmodemXXXX<br />(x, XXXXは任意の数字)|
|Baudrate|9600|
|Data bits|8|
|Parity|none|
|Stop bits|1|
|Flow control|none|

### サンプルアプリケーション ([app.rb](https://github.com/mimaki/stm32l476-mbed-mruby/blob/master/app.rb)) の出力例

```
mruby!
Hello, mruby!
Hello, Hello, mruby!
Hello, Hello, Hello, mruby!
Hello, Hello, Hello, Hello, mruby!
Hello, Hello, Hello, Hello, Hello, mruby!
Hello, Hello, Hello, Hello, Hello, Hello, mruby!
Hello, Hello, Hello, Hello, Hello, Hello, Hello, mruby!
Hello, Hello, Hello, Hello, Hello, Hello, Hello, Hello, mruby!
Hello, Hello, Hello, Hello, Hello, Hello, Hello, Hello, Hello, mruby!
2
42.195
mrubymruby
[1, 4, 7, 1, 4, 7]
```

### mrubyアプリケーションの更新

app.rbを変更した後、```make```を実行して、ビルド結果の stm32l476-mbed-mruby.bin を NUCLEO-L476RG に書き込むことで、mrubyアプリケーションを更新することができます。


---
# ライセンス

本ソフトウェアはMITライセンスのもとで公開しています。[LICENSE](https://github.com/mimaki/stm32l476-mbed-mruby/blob/master/LICENSE)を参照して下さい。

# mrubyに関連するサイト
[mruby.org](http://www.mruby.org)

[特定非営利活動法人mrubyフォーラム](http://forum.mruby.org)

