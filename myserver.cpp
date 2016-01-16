#include <myserver.h>
#include <QStringList>

MyServer::MyServer(int nPort, QWidget* pwgt) : QWidget (pwgt), nextBlockSize(0), version(0)
{
    tcpServer = new QTcpServer(this);

    if ( !tcpServer->listen(QHostAddress::Any, nPort)) {

        QMessageBox::critical(0, "Server Error", "Unable to start the server:" + tcpServer->errorString ());
        tcpServer->close ( );
        return;
    }

    connect (tcpServer, SIGNAL (newConnection() ) , this, SLOT (slotNewConnection()));

    sdb = QSqlDatabase::addDatabase("QSQLITE");
    sdb.setDatabaseName(QCoreApplication::applicationDirPath() + "/project2.sqlite");

    QVBoxLayout* pvbxLayout = new QVBoxLayout;
    pvbxLayout->addWidget(new QLabel("<H1>Server</H1>")) ;
    setLayout(pvbxLayout);
}

void MyServer::slotNewConnection ()
{
    QTcpSocket* pClientSocket = tcpServer->nextPendingConnection();

    connect(pClientSocket, SIGNAL(disconnected()), pClientSocket , SLOT(deleteLater()));

    connect(pClientSocket, SIGNAL(readyRead()), this , SLOT(slotReadClient()));
}

void MyServer::slotReadClient()
{

    QTcpSocket* pClientSocket = (QTcpSocket*)sender();

    QDataStream in(pClientSocket);
    in.setVersion(QDataStream::Qt_5_3);


    for(;;){

        if(!nextBlockSize){
            if(pClientSocket->bytesAvailable() < sizeof(quint32)) {
                break;
            }

            in >> nextBlockSize;
        }

        if(pClientSocket->bytesAvailable() < nextBlockSize) {
            break;
        }


        QString request;
        in >> request;

        QStringList req_param = request.split(" ");

        if(req_param.at(0) == "GETCURRENTVERSION")
            sendCurrVers(pClientSocket);

        if(req_param.at(0) == "GETSUBJECTS")
            sendArrayToClient(pClientSocket,getSubjects());

        if(req_param.at(0) == "GETTHEMES")
            sendArrayToClient(pClientSocket,getThemes(req_param.at(1)));

        if(req_param.at(0) == "GETPICTURES")
            sendArrayToClient(pClientSocket,getPictures(req_param.at(1),req_param.at(2)));

        if(req_param.at(0) == "GETIMAGE")
            sendImageToClient(pClientSocket,getImage(req_param.at(1)));

        if(req_param.at(0) == "PUTSUBJECT"){
            putSubject(req_param.at(1));
            version++;
        }
        if(req_param.at(0) == "PUTTHEME"){
            putTheme(req_param.at(1),req_param.at(2));
            version++;
        }
        if(req_param.at(0) == "PUTIMAGE")
        {
            QImage img;

            in >> img;

            quint32 k = putImage(img,req_param.at(1),req_param.at(2), req_param.at(3));
            version++;
            QList<QString> lst;
            lst << QString("%1").arg(k);
            sendArrayToClient(pClientSocket,lst);
        }
        if(req_param.at(0) == "DELETESUBJECT"){
            deleteSubject(req_param.at(1));
            version++;
        }
        if(req_param.at(0) == "DELETETHEME"){
            deleteTheme(req_param.at(1),req_param.at(2));
            version++;
        }
        if(req_param.at(0) == "DELETEIMAGE"){
            deleteImage(req_param.at(1));
            version++;
        }
        if(req_param.at(0) == "SWAPIMAGE"){
            QImage img;

            in >> img;

            swapImage(img,req_param.at(1));
            version++;
        }

        nextBlockSize = 0;
    }
}

void MyServer::sendCurrVers(QTcpSocket* pSocket)
{

    QByteArray arrBlock;

    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);

    out << quint32(0) << version;
    out.device()->seek(0);
    out << quint32(arrBlock.size() - sizeof(quint32));

    pSocket->write(arrBlock);
}

void MyServer::sendArrayToClient(QTcpSocket* pSocket, QList<QString> lst_str)
{

    QByteArray arrBlock;

    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);

    out << quint32(0) << lst_str;
    out.device()->seek(0);
    out << quint32(arrBlock.size() - sizeof(quint32));

    pSocket->write(arrBlock);
}

void MyServer::sendImageToClient(QTcpSocket* pSocket, QImage img)
{

    QByteArray arrBlock;

    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);

    out << quint32(0) << img;
    out.device()->seek(0);
    quint64 size = quint64(arrBlock.size() - sizeof(quint32));
    out << quint32(arrBlock.size() - sizeof(quint32));

    quint64 k = pSocket->write(arrBlock);
    k = 0;
}

QList<QString> MyServer::getSubjects(){

    sdb.open();
    QSqlQuery query("SELECT name_subject FROM SUBJECTS");

    QList<QString> subj_lst;
    while (query.next())
        subj_lst << query.value(0).toString();
    sdb.close();
    return subj_lst;
}

QList<QString> MyServer::getThemes(QString subject){

    sdb.open();
    QString qr_str = QString("SELECT name_theme FROM THEMES WHERE id_subj = "
                             "(SELECT id FROM SUBJECTS WHERE name_subject = '%1')").arg(subject);
    QSqlQuery query(qr_str);

    QList<QString> thm_lst;
    while (query.next())
             thm_lst << query.value(0).toString();
    sdb.close();
    return thm_lst;
}

QList<QString> MyServer::getPictures(QString subject,QString theme){

    sdb.open();
    QString qr_str = QString("SELECT name_pic FROM PICTURES WHERE id_theme ="
                             " (SELECT id FROM THEMES WHERE name_theme = '%1' and id_subj ="
                             " (SELECT id FROM SUBJECTS WHERE name_subject = '%2'))").arg(theme).arg(subject);
    QSqlQuery query(qr_str);

    QList<QString> pic_lst;
    while (query.next())
             pic_lst << query.value(0).toString();
    sdb.close();
    return pic_lst;
}

QImage MyServer::getImage(QString img_nm){
    QImage img(QCoreApplication::applicationDirPath() + "/pictures/" + img_nm);
    return img;
}

void MyServer::putSubject(QString subj_nm){

    sdb.open();
    QString qr_str = QString("INSERT INTO SUBJECTS (name_subject) VALUES ('%1')").arg(subj_nm);
    QSqlQuery query(qr_str);
    sdb.close();
}

void MyServer::putTheme(QString subj_nm, QString thm_nm){

    sdb.open();
    QString qr_str = QString("INSERT INTO THEMES (name_theme, id_subj) "
                             "VALUES ('%1',(SELECT id FROM SUBJECTS WHERE name_subject = '%2'))").arg(thm_nm).arg(subj_nm);
    QSqlQuery query(qr_str);
    sdb.close();
}

quint32 MyServer::putImage(QImage img, QString subj_nm, QString thm_nm, QString imgExt){

    sdb.open();

    QString qr_str = QString("SELECT MAX(id) FROM PICTURES");
    QSqlQuery query(qr_str);
    query.next();
    quint32 id = query.value(0).toInt();
    ++id;
    QString path = QCoreApplication::applicationDirPath() + QString("/pictures/%1.%2").arg(id).arg(imgExt.toLower()) ;
    img.save(path);

    qr_str = QString("INSERT INTO PICTURES (id, id_theme, name_pic) "
                     "VALUES (%1,(SELECT id FROM THEMES WHERE name_theme = '%2' and id_subj = "
                     "(SELECT id FROM SUBJECTS WHERE name_subject = '%3')),'%4.%5')").arg(id).arg(thm_nm).arg(subj_nm).arg(id).arg(imgExt.toLower());
    QSqlQuery query1(qr_str);
    sdb.close();

    return id;
}

void MyServer::deleteSubject(QString subj_nm){
    sdb.open();
    QSqlQuery query(QString("DELETE FROM SUBJECTS WHERE name_subject = '%1'").arg(subj_nm));
    sdb.close();
}

void MyServer::deleteTheme(QString subj_nm, QString thm_nm){
    sdb.open();
    QSqlQuery query(QString("DELETE FROM THEMES WHERE name_theme = '%1' and id_subj = "
                            "(SELECT id FROM SUBJECTS WHERE name_subject = '%2')").arg(thm_nm).arg(subj_nm));
    sdb.close();
}

void MyServer::deleteImage(QString img_nm){

    sdb.open();
    QSqlQuery query(QString("DELETE FROM PICTURES WHERE name_pic = '%1'").arg(img_nm));
    QFile(QCoreApplication::applicationDirPath() + "/picture/" + img_nm).remove();
    sdb.close();
}

void MyServer::swapImage(QImage new_img, QString old_img_nm){
    QFile(QCoreApplication::applicationDirPath() + "/picture/" + old_img_nm).remove();
    new_img.save(QCoreApplication::applicationDirPath() + "/picture/" + old_img_nm);
}
