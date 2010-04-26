mugo

mugoは囲碁の棋譜エディターです。

以下のフォーマット形式をサポートしています。
読込 - sgf / ugf / ngf/ gib
保存 - sgf

mugoはGTPプロトコルを使用して囲碁プログラムと通信することが出来ます。


mugoを起動するにはQt frameworkが必要です。
Qt frameworkは以下のサイトからダウンロード可能です。

Qt web site: http://qt.nokia.com/
Qt framework for Windows  : http://qt.nokia.com/downloads/windows-cpp
Qt framework for Mac      : http://qt.nokia.com/downloads/mac-os-cpp
Qt framework for Linux/X11: http://qt.nokia.com/downloads/linux-x11-cpp


ビルド

mugoをソースからビルドするには、Qt SDKとC++ Boost Librariesが必要です。
qmakeコマンドでmugo.proからMakefileを作成し、makeコマンドでビルドしてください。
