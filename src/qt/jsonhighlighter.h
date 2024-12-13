#ifndef JSONHIGHLIGHTER_H
#define JSONHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>

class JsonHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    JsonHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
    {
        keyFormat.setForeground(QColor("#56B6C2")); // Light Blue
        keyFormat.setFontWeight(QFont::Bold);

        // Format for strings
        stringFormat.setForeground(QColor("#98C379")); // Green

        // Format for numbers
        numberFormat.setForeground(QColor("#D19A66")); // Orange

        // Format for braces
        braceFormat.setForeground(QColor("#ABB2BF")); // Gray

        // Format for errors
        errorFormat.setForeground(QColor("#E06C75")); // Red

        // Regular expressions for JSON tokens without quotes
        keyPattern = QRegularExpression("\\b\\w+\\b(?=\\s*:)");                 // Keys
        valueStringPattern = QRegularExpression(":\\s*[^\\d\\[{\\]}][^,]*");    // Non-numeric values
        stringPattern = QRegularExpression("\"[^\"]*\"|(\\b[a-fA-F0-9]{64}\\b)"); // Strings and unquoted hashes
        numberPattern = QRegularExpression("\\b\\d+\\b");                       // Numbers
        numberPattern = QRegularExpression("\\b\\d+\\b");                       // Numbers
        bracePattern = QRegularExpression("[{}\\[\\]]");                        // Braces
    }

protected:
    void highlightBlock(const QString &text) override {
        // Apply formats based on patterns
        applyFormat(text, keyPattern, keyFormat);
        applyFormat(text, valueStringPattern, stringFormat);
        applyFormat(text, stringPattern, stringFormat);
        applyFormat(text, numberPattern, numberFormat);
        applyFormat(text, bracePattern, braceFormat);

        if (text.contains("error", Qt::CaseInsensitive)) {
            setFormat(0, text.length(), errorFormat);
        }
    }

private:
    QTextCharFormat keyFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat braceFormat;
    QTextCharFormat errorFormat;

    QRegularExpression keyPattern;
    QRegularExpression valueStringPattern;
    QRegularExpression stringPattern;
    QRegularExpression numberPattern;
    QRegularExpression bracePattern;

    void applyFormat(const QString &text, const QRegularExpression &pattern, const QTextCharFormat &format) {
        QRegularExpressionMatchIterator i = pattern.globalMatch(text);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            setFormat(match.capturedStart(), match.capturedLength(), format);
        }
    }
};

#endif // JSONHIGHLIGHTER_H
