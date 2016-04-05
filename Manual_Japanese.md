# 印刷 #
![http://sikusiku.sakura.ne.jp/mugo/manual/jp/print1.png](http://sikusiku.sakura.ne.jp/mugo/manual/jp/print1.png)
![http://sikusiku.sakura.ne.jp/mugo/manual/jp/print2.png](http://sikusiku.sakura.ne.jp/mugo/manual/jp/print2.png)

# ファイル名や印刷でのマクロに関して #

新規保存時のファイル名の設定や、印刷時のヘッダー／フッターの文字列では、%で囲うことでSGFのプロパティーが使用可能。<br />
使用可能なプロパティーと意味は下記の表を参照。<br />
現在(バージョン1.0.0）は、保存時と印刷時でのマクロ展開が別実装になっており、保存時に使えるプロパティーは印刷時と比べて少なくなっています。<br />
次回のバージョンで印刷時の実装に統一します。<br />

![http://sikusiku.sakura.ne.jp/mugo/manual/jp/sgfmacro_optiondialog.png](http://sikusiku.sakura.ne.jp/mugo/manual/jp/sgfmacro_optiondialog.png)

|GM|1固定。SGFの種別で1は囲碁のデータであることを表す。|
|:-|:---------------------------|
|FF|4固定。SGFのバージョン。              |
|AP|アプリケーション名で mugo:1.0.0 のような文字列。後ろはバージョン番号のため、使用しているバージョンによって変わる。|
|SZ|盤のサイズ。19など。正方形でない盤の場合、25:20などのように出力される。|
|KM|コミ。6.5など。                   |
|HA|置き石の数。0とか2とか9とか。            |
|RU|ルール。<s>現在はJapanese固定。</s>違った。特に処理していなかったので、読み込んだ時の値がそのまま出力される。mugoで新規作成したsgfの場合、空文字|
|PW|白番のプレイヤー名                   |
|WR|白番のプレイヤーの段級位                |
|WT|白番のプレイヤーのチーム                |
|PB|黒番のプレイヤー名                   |
|BR|黒番のプレイヤーの段級位                |
|BT|黒番のプレイヤーのチーム                |
|RE|結果                          |
|TM|持ち時間                        |
|OT|秒読みの設定                      |
|DT|対局日時                        |
|GN|大会名                         |
|RO|ラウンド                        |
|PC|対局場所                        |
|EV|イベント名                       |
|GC|コメント                        |
|AN|注釈                          |
|CP|コピーライト                      |
|ON|布石                          |
|SO|ソース                         |
|US|ユーザー                        |

印刷時には、SGFのプロパティーとは別に以下のマクロも使用可能（保存オプションでは使用不可）<br />
|file|印刷するファイル名のフルパス。保存されていない場合は、無題-1など。|
|:---|:---------------------------------|
|datetime|印刷を実行した瞬間の日付と時間。                  |
|date|印刷を実行した瞬間の日付。                     |
|time|印刷を実行した瞬間の時間。                     |
|page|現在印刷中のページ                         |