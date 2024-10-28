= FrankenPHPを使ってみた
最近FrankenPHPを使用しているので、簡単な紹介になります。

== FrankenPHPとは
FrankenPHP@<fn>{official}とはGo言語で書かれた、新しいPHPサーバーです。
//footnote[official][@<href>{https://frankenphp.dev/}]

主な特徴は、

 1. Caddy上に構築されたPHP8.2以降のサーバー
 2. シングルバイナリ
 3. ハイパフォーマンス
 4. HTTP/2およびHTTP/3のサポート
 5. Early Hintsのサポート
 6. Mercureを利用したWebSocket
 7. Let's EncryptまたはZeroSSLを利用した証明書の自動更新

また、Laravel OctaneによりLaravelは完全にサポートされています。

== FrankenPHPを使う
PHPサーバーを構築する際、一般的には次のような構成になると思います。

 * Apache + mod_php
 * Nginx + FPM

FrankenPHPの場合は、FrankenPHPのみで動作します。

Dockerを利用する場合は、次のコマンドで実行できます。

//cmd{
docker run -v $PWD:/app/public -p 80:80 -p 443:443 -p 443:443/udp \
    dunglas/frankenphp
//}

1つのコンテナだけで済むので、Amazon ECSなどとの相性はいいです。

=== WordPressをFrankenPHPで動かしてみる
WordPressを動かす場合、次のようなDockerfileとcompose.ymlになります。

//list[frankendockerfile][Dockerfile]{
FROM dunglas/frankenphp:php8.3
RUN docker-php-ext-install pdo_mysql mysqli
COPY wordpress /app/public
//}

//list[frankencompose][compose.yml]{
services:
    php:
        build: .
        ports:
            - "80:80"
            - "443:443"
            - "443:443/udp"
    mysql:
        image: 'mysql/mysql-server:8.0'
        environment:
            MYSQL_ROOT_PASSWORD: 'password'
            MYSQL_ROOT_HOST: '%'
            MYSQL_DATABASE: 'mysql'
            MYSQL_USER: 'mysql'
            MYSQL_PASSWORD: 'password'
//}

== パフォーマンス比較
DockerでFrankenPHP、FPM + Nginx、Apacheの環境に対して、WordPressの表示測定しました。
測定条件はNginxのFastCGIまわりの設定を除き、サーバーもWordPressもデフォルトの設定で、siegeを用い記事の表示を60秒間、同接100と200ので測定しました。

実行に使用したDockerの構成や、省略した詳しい結果は @<href>{https://neln.net/a/1730039093} を参照してください。

//table[100][同接100]{
.	FrankenPHP	FPM	Apache
-------------------------------------------------------------
Transactions	28000 hits	23366 hits	21972 hits
Availability	100.00%	100.00%	100.00%
Response time	0.21 secs	0.26 secs	0.27 secs
Transaction rate	465.04 trans/sec	383.17 trans/sec	365.04 trans/sec
Longest transaction	2.27	2.89	19.78
//}

//table[200][同接200]{
.	FrankenPHP	FPM	Apache
-------------------------------------------------------------
Transactions	27807 hits	23344 hits	6144 hits
Availability	100.00%	100.00%	84.64%
Response time	0.42 secs	0.50 secs	1.01 secs
Transaction rate	456.30 trans/sec	382.81 trans/sec	181.99 trans/sec
Longest transaction	4.34	5.9	23.74
//}


== vs. FPM
パフォーマンス面ではやや劣るものの、ほぼ同じようなスコアになりました。

一方構築面ではFPMコンテナのほか、Nginxなどのコンテナが必要となり、常に2つのコンテナを動かすことになります。
同じコンテナ内に収める方法もありますが、1プロセス1コンテナのベストプラクティスに反します。
また、NginxにFastCGIの設定を書いたファイルが必要になるなど、動かすまでに少し手間がかかります。

== vs. Apache
パフォーマンス面では、FrankenPHPや、FPMと比べると大きく劣る結果になりました。
とくに同接200では唯一、正常応答が100%になっていません。

一方構築面ではFrankenPHP同様、Apache自身がPHPを実行するためコンテナは1つになります。
またとくに設定しなくても、ドキュメントルートにPHPファイルを置くだけで実行できます。

== FrankenPHPの問題
一見、FrankenPHPは便利そうに見えますが、既知の問題@<fn>{issues}もあります。

//footnote[issues][@<href>{https://frankenphp.dev/docs/known-issues/}]

2024年10月時点では、PHP拡張機能のimapとnewrelicがサポートされていません。また、ext-opensslとparallelは不具合が報告されています。
PHP拡張機能以外では、@<code>{get_browser()}関数のパフォーマンス低下などがあります。
いずれの場合も回避策はありますが、ほかと比べると安定性や互換性の面がやや気になるところです。

また、FrankenPHPは共有メモリを利用して高速化しています。
状況によってはメモリーリークなどが発生します。

== おわりに
FrankenPHPは従来のApache、FPMとは異なるアプローチでPHP用のWebサーバーです。
サクッとPHPサーバーを構築できパフォーマンスもよく、使いやすいと思います。
安定性や互換性によほどシビアな要求がなければ、十分に実戦投入できると思います。

//embed[latex]{
\vfill
//}

===== 発行所
Neln

===== 著者
ゆずねり@<br>{}
@<href>{https://neln.net}@<br>{}
X @<href>{https://twitter.com/yuzneri, @yuzneri}@<br>{}
@<icon>{x}

===== 発行日
2024年11月2日
