#ifndef BLOCKBROWSER_H
#define BLOCKBROWSER_H

#include <QDialog>
#include <QObject>
#include <QTextEdit>
#include "walletmodel.h"
#include "clientmodel.h"
#include "jsonhighlighter.h"

namespace Ui {
class BlockBrowser;
}

#define WEBAPIHOST  tr("https://explorer.chesscoin032.com")

class BlockBrowser : public QDialog
{
    Q_OBJECT

public:
    explicit BlockBrowser(QWidget *parent = 0);
    ~BlockBrowser();

    void setClientModel(ClientModel *model);
    void setWalletModel(WalletModel *model);

private:
    bool isValidHash(const QString &hash);
    bool validateSearchText(int searchOption);

    void fetchBlockInfo(const QString &text, int searchOption, QTextEdit *resultContent);
    void fetchBlockInfoWithHeight(const QString& height, QTextEdit *resultContent);

    QString prettyPrintJson(const QString &jsonString);
    QString scrapeChesscoin032Api(const QString& url);

private slots:
    void on_btnFindBlock_clicked();
    void on_btnCopy_clicked();
    void on_btnClear_clicked();
    void setNumBlocks(int count, int countOfPeers);

private:
    Ui::BlockBrowser *ui;
    WalletModel *wModel;
    ClientModel *cModel;
    JsonHighlighter *highlighter;
};

#endif // BLOCKBROWSER_H
