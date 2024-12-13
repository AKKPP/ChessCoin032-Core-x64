#include "blockbrowser.h"
#include "ui_blockbrowser.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#include <QRegularExpression>
#else
#include <QRegExp>
#endif
#ifdef QT_SUPPORTSSL
#include <QNetworkAccessManager>
#include <QNetworkReply>
#else
#include "curlnet.h"
#endif

#include <QClipboard>
#include <QJsonDocument>
#include <QJsonObject>


BlockBrowser::BlockBrowser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BlockBrowser)
{
    ui->setupUi(this);

    highlighter = new JsonHighlighter(ui->txtContent->document());

#ifdef Q_OS_WIN
    resize(960, 500);
    setMinimumSize(960, 500);
#elif defined Q_OS_MAC
    resize(960, 620);
    setMinimumSize(960, 620);
#else
    resize(960, 620);
    setMinimumSize(960, 620);
#endif
}

BlockBrowser::~BlockBrowser()
{
    delete ui;
}

void BlockBrowser::setClientModel(ClientModel *model)
{
    if (model)
    {
        cModel = model;
        setNumBlocks(cModel->getNumBlocks(), 0);
        connect(cModel, SIGNAL(numBlocksChanged(int,int)), this, SLOT(setNumBlocks(int,int)));
    }
}

void BlockBrowser::setWalletModel(WalletModel *model)
{
    wModel = model;
}

void BlockBrowser::setNumBlocks(int count, int countOfPeers)
{
    Q_UNUSED(countOfPeers);
    ui->lblHeight->setText(QString::number(count));
}


bool BlockBrowser::isValidHash(const QString &hash)
{
    // Regular expression to validate a 64-character hexadecimal hash

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    QRegularExpression regex("^[a-fA-F0-9]{64}$");
    return regex.match(hash).hasMatch();
#else
    QRegExp regex("^[a-fA-F0-9]{64}$");
    return regex.exactMatch(hash);
#endif
}

bool BlockBrowser::validateSearchText(int searchOption)
{
    bool result = true;

    switch (searchOption)
    {
    case 0:
        {
            //int blockheight = cModel->getNumBlocks() + 5;
            int inputheight = ui->searchText->text().toInt();
            if (inputheight <= 0 || inputheight > 500000000)
            {
                ui->searchText->setValid(false);
                result = false;
            }
        }
        break;
    case 1:
    case 2:
        {
            if (!isValidHash(ui->searchText->text())) {
                ui->searchText->setValid(false);
                result = false;
            }
        }
        break;
    case 3:
        {
            if (!ui->searchText->hasAcceptableInput() || (wModel && !wModel->validateAddress(ui->searchText->text())))
            {
                ui->searchText->setValid(false);
                result = false;
            }
        }
        break;
    default:
        result = false;
        break;
    }

    return result;
}

QString BlockBrowser::prettyPrintJson(const QString &jsonString)
{
    int indentLevel = 0;
    QString prettyJson;
    const int indentSize = 4;

    for (int i = 0; i < jsonString.size(); ++i) {
        QChar c = jsonString.at(i);

        // Add new lines and indentation for readability
        switch (c.unicode()) {
            case '{':
            case '[':
                prettyJson.append(c);
                prettyJson.append("\n");
                prettyJson.append(QString(indentLevel += indentSize, ' '));
                break;
            case '}':
            case ']':
                prettyJson.append("\n");
                prettyJson.append(QString(indentLevel -= indentSize, ' '));
                prettyJson.append(c);
                break;
            case ',':
                prettyJson.append(c);
                prettyJson.append("\n");
                prettyJson.append(QString(indentLevel, ' '));
                break;
            case ':':
                prettyJson.append(c);
                prettyJson.append(" ");
                break;
            default:
                prettyJson.append(c);
        }
    }
    return prettyJson;
}

#ifdef QT_SUPPORTSSL
QString BlockBrowser::scrapeChesscoin032Api(const QString& url)
{
    QString resultContent = "";

    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);

    // Use QEventLoop to wait for the reply
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();  // Blocks until the reply is finished

    // Check for errors
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray response = reply->readAll();
        resultContent = prettyPrintJson(response).replace("\"", "");
    } else {
        resultContent = reply->errorString();
    }

    reply->deleteLater();  // Clean up the reply
    return resultContent;
}
#else
QString BlockBrowser::scrapeChesscoin032Api(const QString& url)
{
    QString resultContent = prettyPrintJson(sendWebRequest(url)).replace("\"", "");
    return resultContent;
}
#endif

void BlockBrowser::fetchBlockInfo(const QString &text, int searchOption, QTextEdit *resultContent)
{
    QString url;
    switch (searchOption)
    {
    case 1:
        url = QString("%1/api/getblock?hash=%2").arg(WEBAPIHOST, text);
        break;
    case 2:
        url = QString("%1/ext/gettx/%2").arg(WEBAPIHOST, text);
        break;
    case 3:
        url = QString("%1/ext/getaddress/%2").arg(WEBAPIHOST, text);
        break;
    }

    QString result = scrapeChesscoin032Api(url);
    resultContent->setPlainText(result);
}

void BlockBrowser::fetchBlockInfoWithHeight(const QString& height, QTextEdit *resultContent)
{
    QString url = QString("%1/api/getblockhash?index=%2").arg(WEBAPIHOST, height);
    QString hash = scrapeChesscoin032Api(url);

    if (!isValidHash(hash))
    {
        resultContent->setPlainText(hash);
        return;
    }

    url = QString("%1/api/getblock?hash=%2").arg(WEBAPIHOST, hash);
    QString result = scrapeChesscoin032Api(url);

    resultContent->setPlainText(result);
}

void BlockBrowser::on_btnFindBlock_clicked()
{
    ui->cmbSearchFilter->setEnabled(false);
    ui->searchText->setEnabled(false);
    ui->btnFindBlock->setEnabled(false);
    ui->txtContent->clear();

    int opt = ui->cmbSearchFilter->currentIndex();
    if (!validateSearchText(opt))
    {
        ui->cmbSearchFilter->setEnabled(true);
        ui->searchText->setEnabled(true);
        ui->btnFindBlock->setEnabled(true);
        return;
    }

    if (opt == 0)
    {
        fetchBlockInfoWithHeight(ui->searchText->text(), ui->txtContent);
    }
    else if (opt > 0 && opt < 4)
    {
        fetchBlockInfo(ui->searchText->text(), opt, ui->txtContent);
    }
    else
    {
        ui->txtContent->setPlainText("Error: Not support");
    }

    ui->cmbSearchFilter->setEnabled(true);
    ui->searchText->setEnabled(true);
    ui->btnFindBlock->setEnabled(true);
}


void BlockBrowser::on_btnCopy_clicked()
{
    QString textToCopy = ui->txtContent->toPlainText();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(textToCopy);
}

void BlockBrowser::on_btnClear_clicked()
{
    ui->searchText->clear();
    ui->txtContent->clear();
}

