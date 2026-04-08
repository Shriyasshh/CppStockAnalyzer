#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QString>

// --- NEW CHART HEADERS ---
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>

#include <iostream>
#include <string>
#include <cstdlib>
#include <laserpants/dotenv/dotenv.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main(int argc, char *argv[]) {
    // Enable High DPI scaling for crisp fonts on Windows
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

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
    window.setMinimumSize(500, 600); // Made window taller for the chart

    // 4. Create UI Elements
    QVBoxLayout* mainLayout = new QVBoxLayout(&window);
    
    QHBoxLayout* inputLayout = new QHBoxLayout();
    QLabel* symbolLabel = new QLabel("Select Stock:");
    QComboBox* symbolCombo = new QComboBox();
    symbolCombo->addItems({"AAPL", "MSFT", "GOOGL", "AMZN", "TSLA", "META", "NVDA"});
    QPushButton* fetchButton = new QPushButton("Fetch & Compare");
    
    inputLayout->addWidget(symbolLabel);
    inputLayout->addWidget(symbolCombo);
    inputLayout->addWidget(fetchButton);

    QLabel* resultLabel = new QLabel("Select a stock to see its price.");
    resultLabel->setAlignment(Qt::AlignCenter);
    QFont font = resultLabel->font();
    font.setPointSize(12);
    font.setBold(true);
    resultLabel->setFont(font);

    QLabel* compareLabel = new QLabel("");
    compareLabel->setAlignment(Qt::AlignCenter);
    QFont compareFont = compareLabel->font();
    compareFont.setPointSize(10);
    compareLabel->setFont(compareFont);

    // --- NEW CHART UI SETUP ---
    QChart* chart = new QChart();
    chart->setTitle("7-Day Price Trend (Simulated History + Live Current)");
    chart->legend()->hide();
    chart->setAnimationOptions(QChart::SeriesAnimations); // Adds a nice drawing animation
    
    QChartView* chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(300);

    // Add everything to the layout
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(resultLabel);
    mainLayout->addWidget(compareLabel);
    mainLayout->addWidget(chartView); // Add the chart to the bottom

    // 5. State variables
    QString lastSymbol = "";
    double lastPrice = 0.0;
    bool hasPrevious = false;

    // 6. Connect Button Click
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

                    QString resultText = QString(
                        "%1 | $%2 | Change: %3% %4"
                    )
                    .arg(symbolQStr)
                    .arg(current_price, 0, 'f', 2)
                    .arg(percent_change, 0, 'f', 2) 
                    .arg(trend);

                    resultLabel->setText(resultText);

                    if (percent_change > 0) resultLabel->setStyleSheet("color: #28a745;"); // Green
                    else resultLabel->setStyleSheet("color: #dc3545;"); // Red

                    // Comparison Logic
                    if (hasPrevious) {
                        double priceDiff = current_price - lastPrice;
                        QString compText = QString("Comparison: %1 is ").arg(symbolQStr);

                        if (priceDiff > 0) {
                            compText += QString("$%1 more expensive than %2 ($%3).")
                                        .arg(priceDiff, 0, 'f', 2).arg(lastSymbol).arg(lastPrice, 0, 'f', 2);
                        } else if (priceDiff < 0) {
                            compText += QString("$%1 cheaper than %2 ($%3).")
                                        .arg(-priceDiff, 0, 'f', 2).arg(lastSymbol).arg(lastPrice, 0, 'f', 2);
                        }
                        compareLabel->setText(compText);
                    }
                    
                    lastSymbol = symbolQStr;
                    lastPrice = current_price;
                    hasPrevious = true;

                    // --- NEW CHART DRAWING LOGIC ---
                    chart->removeAllSeries(); // Clear the old line
                    QLineSeries* series = new QLineSeries();
                    
                    // Generate mock historical data based on current price for visual effect
                    // Day 0 to Day 5 are random fluctuations, Day 6 is the real live price
                    double start_price = current_price * 0.95; // Start 5% lower just for the visual
                    for(int i = 0; i < 6; i++) {
                        // Create a slight random walk up to the real price
                        double random_modifier = ((rand() % 100) / 100.0) * 0.04 - 0.02; // +/- 2%
                        start_price += (start_price * random_modifier);
                        series->append(i, start_price);
                    }
                    // Day 6 is the actual live price you just fetched
                    series->append(6, current_price); 

                    // Add the line to the chart and reset the axes to fit
                    chart->addSeries(series);
                    chart->createDefaultAxes();

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

    // 7. Show Window and Run
    window.show();
    return app.exec();
}
