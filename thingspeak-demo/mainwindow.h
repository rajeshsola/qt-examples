#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariantMap>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void on_pbSettings_clicked();
    void on_pbPublish_clicked();
    void on_pbRetrieve_clicked();

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *restclient;
    QNetworkRequest request;
    QNetworkReply *reply;
    bool locked;
    void updateChannelList(QByteArray&);
    void updateChannelKeys(QByteArray&);
    void displayFeeds(QByteArray&);
    QVariantList chlist;
    QVariantMap chmap;
private slots:
    void replyFinished(QNetworkReply*);
    void on_pbPublishPost_clicked();
    void on_pbListChanels_clicked();
    void on_cbListChannels_currentIndexChanged(const QString &arg1);
    //void on_cbListChannels_currentIndexChanged(int index);
    void on_pbChannelView_clicked();
    void on_pbChannelClear_clicked();
    void on_pbChannelDelete_clicked();
    void on_pbChannelCreate_clicked();
};

#endif // MAINWINDOW_H
