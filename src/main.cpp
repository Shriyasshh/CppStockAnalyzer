#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QString>

#include <iostream>
#include <string>
#include <cstdlib>
#include <laserpants/dotenv/dotenv.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main(int argc, char *argv[]) {
    // 1. Load Environment Variables
    try {
        dotenv::init();
    } catch (const std::exception& e) {
        std::cerr << "Warning: Could not load .env file. " << e.what() << std::endl;
    }

    const char* env_p = std::getenv("API_KEY");
    if (!env_p) {
        std::cerr << "Error: API_KEY not found in environment." << std::endl;
        return 1;
    }
    std::string api_key = env_p;

    // 2. Initialize Qt Application
    QApplication app(argc, argv);

    // 3. Setup Main Window
    QWidget window;
    window.setWindowTitle("Stock Market Tracker");
    window.setMinimumSize(450, 250);

    // 4. Create UI Elements
    QVBoxLayout* mainLayout = new QVBoxLayout(&window);
    
    QHBoxLayout* inputLayout = new QHBoxLayout();
    QLabel* symbolLabel = new QLabel("Select Stock:");
    
    // Replaced the text input with a Dropdown Box (Combo Box)
    QComboBox* symbolCombo = new QComboBox();
    symbolCombo->addItems({"AAPL", "MSFT", "GOOGL", "AMZN", "TSLA", "META", "NVDA"});
    
    QPushButton* fetchButton = new QPushButton("Fetch & Compare");
    
    inputLayout->addWidget(symbolLabel);
    inputLayout->addWidget(symbolCombo);
    inputLayout->addWidget(fetchButton);

    // Label for the current search result
    QLabel* resultLabel = new QLabel("Select a stock to see its price.");
    resultLabel->setAlignment(Qt::AlignCenter);
    QFont font = resultLabel->font();
    font.setPointSize(11);
    resultLabel->setFont(font);

    // Label for the comparison result
    QLabel* compareLabel = new QLabel("");
    compareLabel->setAlignment(Qt::AlignCenter);
    QFont compareFont = compareLabel->font();
    compareFont.setPointSize(10);
    compareFont.setItalic(true);
    compareLabel->setFont(compareFont);

    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(resultLabel);
    mainLayout->addWidget(compareLabel);

    // 5. State variables to remember the previous search
    QString lastSymbol = "";
    double lastPrice = 0.0;
    bool hasPrevious = false;

    // 6. Connect Button Click to Fetch and Compare Logic
    QObject::connect(fetchButton, &QPushButton::clicked, [&]() {
        QString symbolQStr = symbolCombo->currentText();
        std::string symbol = symbolQStr.toStdString();

        resultLabel->setText("Fetching data for " + symbolQStr + "...");
        QApplication::processEvents(); 

        std::string url = "https://finnhub.io/api/v1/quote?symbol=" + symbol + "&token=" + api_key;
        cpr::Response r = cpr::Get(cpr::Url{url});

        if (r.status_code == 200) {
            try {
                json data = json::parse(r.text);
                
                if (data.contains("c") && data.contains("pc") && data["c"] != 0) {
                    double current_price = data["c"];
                    double previous_close = data["pc"];
                    double percent_change = ((current_price - previous_close) / previous_close) * 100.0;
                    
                    QString trend = "FLAT -";
                    if (percent_change > 0) trend = "UP ↑";
                    else if (percent_change < 0) trend = "DOWN ↓";

                    // Update Current Result
                    QString resultText = QString(
                        "--- %1 ---\n"
                        "Current Price  : $%2\n"
                        "Daily Change   : %3%\n"
                        "Trend          : %4"
                    )
                    .arg(symbolQStr)
                    .arg(current_price, 0, 'f', 2)
                    .arg(percent_change, 0, 'f', 2) 
                    .arg(trend);

                    resultLabel->setText(resultText);

                    // Comparison Logic
                    if (hasPrevious) {
                        double priceDiff = current_price - lastPrice;
                        QString compText = QString("Comparison: %1 is ").arg(symbolQStr);

                        if (priceDiff > 0) {
                            compText += QString("$%1 more expensive than %2 ($%3).")
                                        .arg(priceDiff, 0, 'f', 2).arg(lastSymbol).arg(lastPrice, 0, 'f', 2);
                            compareLabel->setStyleSheet("color: #d9534f;"); // Red for more expensive
                        } else if (priceDiff < 0) {
                            compText += QString("$%1 cheaper than %2 ($%3).")
                                        .arg(-priceDiff, 0, 'f', 2).arg(lastSymbol).arg(lastPrice, 0, 'f', 2);
                            compareLabel->setStyleSheet("color: #5cb85c;"); // Green for cheaper
                        } else {
                            compText += QString("the exact same price as %1.").arg(lastSymbol);
                            compareLabel->setStyleSheet("color: gray;");
                        }
                        compareLabel->setText(compText);
                    } else {
                        // First time searching, nothing to compare yet
                        compareLabel->setStyleSheet("color: gray;");
                        compareLabel->setText("Search another stock to compare it against " + symbolQStr + ".");
                    }

                    // Save the current search as the "previous" search for next time
                    lastSymbol = symbolQStr;
                    lastPrice = current_price;
                    hasPrevious = true;

                } else {
                    resultLabel->setText("Error: Invalid symbol or no data returned.");
                }
            } catch (const json::parse_error& e) {
                resultLabel->setText("Error: Failed to parse JSON.");
            }
        } else {
            resultLabel->setText(QString("HTTP Error: %1").arg(r.status_code));
        }
    });

    // 7. Show Window and Run Event Loop
    window.show();
    return app.exec();
}
