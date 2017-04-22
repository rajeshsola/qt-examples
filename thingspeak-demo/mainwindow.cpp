#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    restclient = new QNetworkAccessManager(this);
    locked=true;
    QObject::connect(restclient, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply *)));
    on_pbListChanels_clicked();
}
MainWindow::~MainWindow()
{
    delete restclient;
    delete ui;
}
void MainWindow::on_pbSettings_clicked()
{
    if(locked)
    {
        ui->leWRKey->setEnabled(true);
        ui->leRDKey->setEnabled(true);
        ui->leChKey->setEnabled(true);
        ui->leChannelId->setEnabled(true);
        ui->pbSettingsLock->setText("Lock");
        locked=false;
    }
    else
    {
        ui->leWRKey->setEnabled(false);
        ui->leRDKey->setEnabled(false);
        ui->leChKey->setEnabled(false);
        ui->leChannelId->setEnabled(false);
        ui->pbSettingsLock->setText("Unlock");
        locked=true;
    }
}
void MainWindow::on_pbPublish_clicked()
{
    int tval = ui->sbField1->value();
    int hval = ui->sbField2->value();
    int pval = ui->sbField3->value();

    QUrlQuery querystr;
    querystr.addQueryItem("api_key",ui->leWRKey->text());
    querystr.addQueryItem("field1",QVariant(tval).toString());
    querystr.addQueryItem("field2",QVariant(hval).toString());
    querystr.addQueryItem("field3",QVariant(pval).toString());

    QUrl url;
    url.setScheme("http");
    url.setHost("api.thingspeak.com");
    url.setPath("/update");
    url.setQuery(querystr);
    qDebug() << url.toString();

    request.setUrl(url);
    QNetworkReply *reply=restclient->get(request);
    qDebug() << "updated successfully" << reply->readAll();
}
void MainWindow::on_pbPublishPost_clicked()
{
    int value1 = ui->sbField1->value();
    int value2 = ui->sbField2->value();
    int value3 = ui->sbField3->value();

    /*QUrlQuery postdata;
    postdata.addQueryItem("api_key",ui->leWRKey->text());
    postdata.addQueryItem("field1",QVariant(value1).toString());
    postdata.addQueryItem("field2",QVariant(value2).toString());
    postdata.addQueryItem("field3",QVariant(value3).toString());*/

    QVariantMap map;
    map.insert("api_key",ui->leWRKey->text());
    map.insert("field1",QVariant(value1));
    map.insert("field2",QVariant(value2));
    map.insert("field3",QVariant(value3));
    QJsonDocument jsdoc=QJsonDocument::fromVariant(map);
    qDebug() << jsdoc.toJson();

    QUrl url;
    url.setScheme("http");
    url.setHost("api.thingspeak.com");
    url.setPath("/update.json");
    //qDebug() << url.toString() << "\n" << postdata.toString();

    request.setUrl(url);
    //request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    //reply=nwManager->post(request,QVariant(postdata.toString()).toByteArray());
    reply=restclient->post(request,jsdoc.toJson());
    qDebug() << "posted successfully" << reply->readAll();
}
void MainWindow::on_pbRetrieve_clicked()
{
    QUrlQuery querystr;
    querystr.addQueryItem("api_key",ui->leRDKey->text());
    if(ui->sbResults->value()!=0)
        querystr.addQueryItem("results",QVariant(ui->sbResults->value()).toString());
    QUrl url;
    url.setScheme("http");
    url.setHost("api.thingspeak.com");
    url.setPath("/channels/228318/feeds.json");
    url.setQuery(querystr);
    //url.setQuery("api_key="+ui->leRDKey->text());

    QNetworkRequest request;
    request.setUrl(url);

    reply=restclient->get(request);
}
void MainWindow::replyFinished(QNetworkReply * reply)
{
    QByteArray buffer=reply->readAll();
    qDebug() << buffer;
    if(reply->request().url().path().endsWith("feeds.json"))
    {
        displayFeeds(buffer);
    }
    else if(reply->request().url().path().endsWith("channels.json"))
    {
        updateChannelList(buffer);
    }
    else if(reply->request().url().path().endsWith(ui->leChannelId->text()+".json"))
    {
        updateChannelKeys(buffer);
    }
    reply->deleteLater();
}
void MainWindow::updateChannelKeys(QByteArray & buffer)
{
    qDebug() << buffer;
    QJsonDocument jsdoc = QJsonDocument::fromJson(buffer);
    QJsonObject jsobj = jsdoc.object();
    QJsonArray jsarr = jsobj["api_keys"].toArray();
    foreach (const QJsonValue &value, jsarr) {
        QJsonObject jsob = value.toObject();
        if(jsob["write_flag"].toBool())
        {
            ui->leWRKey->setText(jsob["api_key"].toString());
        }
        else
        {
            ui->leRDKey->setText(jsob["api_key"].toString());
        }
     }
}

void MainWindow::displayFeeds(QByteArray & buffer)
{
    qDebug() << buffer;
    QJsonDocument jsdoc = QJsonDocument::fromJson(buffer);
    QJsonObject jsobj = jsdoc.object();
    QJsonValue jschannel = jsobj.value(QString("channel"));
    QVariantMap jsmap = jschannel.toObject().toVariantMap();
    for(QVariantMap::const_iterator iter = jsmap.begin(); iter != jsmap.end(); ++iter) {
        qDebug() << iter.key() << "," << iter.value();
    //ui->twFeeds->append(QVariant(iter.key()).toString()+"==>"+QVariant(iter.value()).toString());
    }
    QJsonArray jsarr = jsobj["feeds"].toArray();
    ui->tableFeeds->clearContents();
    ui->tableFeeds->setColumnCount(5);
    ui->tableFeeds->setRowCount(jsarr.count());
    int k=0;
    foreach (const QJsonValue &value, jsarr) {
        QJsonObject jsob = value.toObject();
        ui->tableFeeds->setItem(k,0,new QTableWidgetItem(QVariant(jsob["entry_id"].toInt()).toString()));
        ui->tableFeeds->setItem(k,1,new QTableWidgetItem(jsob["field1"].toString()));
        ui->tableFeeds->setItem(k,2,new QTableWidgetItem(jsob["field2"].toString()));
        ui->tableFeeds->setItem(k,3,new QTableWidgetItem(jsob["field3"].toString()));
        ui->tableFeeds->setItem(k,4,new QTableWidgetItem(jsob["created_at"].toString()));
        k++;
    }
}
void MainWindow::updateChannelList(QByteArray& buffer)
{
    qDebug() << buffer;
    QJsonDocument jsdoc = QJsonDocument::fromJson(buffer);
    QJsonArray jsarr = jsdoc.array();
    ui->cbListChannels->clear();
    foreach (const QJsonValue &value, jsarr) {
        QJsonObject jsob = value.toObject();
        ui->cbListChannels->addItem(jsob["name"].toString());
        chmap.insert(jsob["name"].toString(),jsob["id"].toVariant());
    }
    for(QVariantMap::const_iterator iter = chmap.begin(); iter != chmap.end(); ++iter)
        qDebug() << iter.key() << "," << iter.value();
}
void MainWindow::on_pbListChanels_clicked()
{
    QUrl url;
    url.setScheme("http");
    url.setHost("api.thingspeak.com");
    url.setPath("/channels.json");

    QUrlQuery querystr;
    querystr.addQueryItem("api_key",ui->leChKey->text());

    url.setQuery(querystr);
    qDebug() << url.toString();

    request.setUrl(url);
    reply=restclient->get(request);
}

void MainWindow::on_cbListChannels_currentIndexChanged(const QString &arg1)
{
    qDebug() << chmap.find(arg1).value().toDouble();
    ui->leChannelId->setText(chmap.find(arg1).value().toString());
    on_pbChannelView_clicked();
}
void MainWindow::on_pbChannelView_clicked()
{
    QUrl url;
    url.setScheme("http");
    url.setHost("api.thingspeak.com");
    url.setPath("/channels/"+ui->leChannelId->text()+".json");

    QUrlQuery querystr;
    querystr.addQueryItem("api_key",ui->leChKey->text());

    url.setQuery(querystr);
    request.setUrl(url);

    reply=restclient->get(request);
}

void MainWindow::on_pbChannelClear_clicked()
{
    QUrl url;
    url.setScheme("http");
    url.setHost("api.thingspeak.com");
    url.setPath("/channels/"+ui->leChannelId->text()+"/feeds");
    url.setQuery("api_key="+ui->leChKey->text());

    //url.setQuery(querystr);
    request.setUrl(url);

    reply=restclient->deleteResource(request);
    qDebug () << "cleared the channel:" << reply->readAll();

}

void MainWindow::on_pbChannelDelete_clicked()
{
    QUrl url;
    url.setScheme("http");
    url.setHost("api.thingspeak.com");
    url.setPath("/channels/"+ui->leChannelId->text());
    url.setQuery("api_key="+ui->leChKey->text());

    request.setUrl(url);

    reply=restclient->deleteResource(request);
    qDebug() << "deleted the channel:" << reply->readAll();
}

void MainWindow::on_pbChannelCreate_clicked()
{
    if(ui->cbChNewEnabled->isChecked())
    {
        QUrl url;
        url.setScheme("http");
        url.setHost("api.thingspeak.com");
        url.setPath("/channels");

        QVariantMap chnew;
        chnew.insert("api_key",ui->leChKey->text());
        chnew.insert("name",ui->leChNewName->text());
        QJsonDocument jsdoc=QJsonDocument::fromVariant(chnew);
        request.setUrl(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
        reply=restclient->post(request,jsdoc.toJson());
        qDebug() << "create a new channel:" << jsdoc.toJson();
        on_pbListChanels_clicked();
    }
}
