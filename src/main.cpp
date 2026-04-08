#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QDateTime>
#include <QRandomGenerator>
#include <QTimer>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QPieSeries>
#include <QPieSlice>
#include <QValueAxis>

#include <iostream>
#include <string>
#include <map>
#include <laserpants/dotenv/dotenv.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class StockTrackerApp : public QWidget {
    Q_OBJECT

public:
    StockTrackerApp(const std::string& apiKey, QWidget *parent = nullptr) 
        : QWidget(parent), api_key(apiKey) {
        setWindowTitle("Stock Market Pro Dashboard");
        setMinimumSize(1000, 700);
        QVBoxLayout* rootLayout = new QVBoxLayout(this);
        stackedWidget = new QStackedWidget(this);
        rootLayout->addWidget(stackedWidget);
        setupHomePage();
        setupLoadingPage();
        setupDashboardPage();
        stackedWidget->setCurrentIndex(0);
    }

private:
    std::string api_key;
    QStackedWidget* stackedWidget;
    QLabel* loadingLabel;
    QLabel* detailsHeaderLabel;
    QLabel* detailsPriceLabel;
    QTextEdit* historySidebar;
    QChart* lineChart;
    QChartView* lineChartView;
    QChart* pieChart;
    QChartView* pieChartView;
    QPieSeries* pieSeries;
    QString currentSymbol = "";
    std::map<QString, double> sessionPrices;
    //std::map<QString, int> fetchCounts; // Tracks how many times each stock was fetched
    void setupHomePage() {
        QWidget* homePage = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(homePage);

        QLabel* title = new QLabel("Select a Company to Analyze");
        title->setAlignment(Qt::AlignCenter);
        title->setStyleSheet("font-size: 24px; font-weight: bold; margin-bottom: 20px;");
        layout->addWidget(title);

        QGridLayout* gridLayout = new QGridLayout();
        std::vector<QString> companies = {
        "AAPL", "MSFT", "GOOGL", "AMZN", 
        "TSLA", "META", "NVDA", "NFLX",
        "AMD",  "INTC", "PYPL", "BABA", 
        "COIN", "DIS",  "SONY", "TSM"  
    };

        int row = 0, col = 0;
        for (const QString& symbol : companies) {
            QPushButton* btn = new QPushButton(symbol);
            btn->setMinimumHeight(100);
            btn->setStyleSheet(
                "QPushButton {"
                "   font-size: 20px; font-weight: bold; background-color: #171515;"
                "   border: 2px solid #ccc; border-radius: 10px;"
                "}"
                "QPushButton:hover { background-color: #232221; border-color: #4a90e2; }"
            );

            connect(btn, &QPushButton::clicked, [this, symbol]() {
                initiateFetch(symbol);
            });

            gridLayout->addWidget(btn, row, col);
            col++;
            if (col > 3) { col = 0; row++; } // 4 columns max
        }

        layout->addLayout(gridLayout);
        layout->addStretch();
        stackedWidget->addWidget(homePage); // Index 0
    }

    void setupLoadingPage() {
        QWidget* loadingPage = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(loadingPage);
        
        loadingLabel = new QLabel("Fetching Data...");
        loadingLabel->setAlignment(Qt::AlignCenter);
        loadingLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #555;");
        
        layout->addWidget(loadingLabel);
        stackedWidget->addWidget(loadingPage); // Index 1
    }

    void setupDashboardPage() {
        QWidget* dashboardPage = new QWidget();
        QHBoxLayout* mainLayout = new QHBoxLayout(dashboardPage);

        QWidget* leftSidebar = new QWidget();
        leftSidebar->setMinimumWidth(300);
        leftSidebar->setMaximumWidth(350);
        QVBoxLayout* leftLayout = new QVBoxLayout(leftSidebar);

        QLabel* historyTitle = new QLabel("Session History");
        historyTitle->setStyleSheet("font-weight: bold; font-size: 14px;");
        historySidebar = new QTextEdit();
        historySidebar->setReadOnly(true);

        pieChart = new QChart();
        pieChart->setTitle("Watchlist Interest (Clicks)");
        pieChart->legend()->setAlignment(Qt::AlignBottom);
        pieSeries = new QPieSeries();
        pieChart->addSeries(pieSeries);
        
        pieChartView = new QChartView(pieChart);
        pieChartView->setRenderHint(QPainter::Antialiasing);
        pieChartView->setMinimumHeight(250);

        leftLayout->addWidget(historyTitle);
        leftLayout->addWidget(historySidebar, 1);
        leftLayout->addWidget(pieChartView, 1);

        QWidget* rightArea = new QWidget();
        QVBoxLayout* rightLayout = new QVBoxLayout(rightArea);

        QHBoxLayout* topInfoLayout = new QHBoxLayout();
        QPushButton* backBtn = new QPushButton("← Back to Home");
        backBtn->setStyleSheet("padding: 10px; font-weight: bold;");
        connect(backBtn, &QPushButton::clicked, [this]() {
            stackedWidget->setCurrentIndex(0); // Go back home
        });

        detailsHeaderLabel = new QLabel("Company Overview");
        detailsHeaderLabel->setStyleSheet("font-size: 24px; font-weight: bold;");
        
        topInfoLayout->addWidget(backBtn);
        topInfoLayout->addStretch();
        topInfoLayout->addWidget(detailsHeaderLabel);
        topInfoLayout->addStretch();

        detailsPriceLabel = new QLabel("Price Info");
        detailsPriceLabel->setAlignment(Qt::AlignCenter);
        detailsPriceLabel->setStyleSheet("font-size: 18px; margin: 15px;");

        lineChart = new QChart();
        lineChart->setTitle("7-Day Simulated Trend vs Live Current");
        lineChart->legend()->hide();
        lineChart->setAnimationOptions(QChart::SeriesAnimations);
        
        lineChartView = new QChartView(lineChart);
        lineChartView->setRenderHint(QPainter::Antialiasing);

        rightLayout->addLayout(topInfoLayout);
        rightLayout->addWidget(detailsPriceLabel);
        rightLayout->addWidget(lineChartView, 1);

        // Add to main layout
        mainLayout->addWidget(leftSidebar);
        mainLayout->addWidget(rightArea);

        stackedWidget->addWidget(dashboardPage); // Index 2
    }

    void initiateFetch(const QString& symbol) {
        currentSymbol = symbol;
        
        loadingLabel->setText(QString("Fetching Market Data for %1...").arg(symbol));
        stackedWidget->setCurrentIndex(1);
        //fetchCounts[symbol]++;
        //updatePieChart();
        QTimer::singleShot(500, this, [this]() {
            performApiFetch();
        });
    }

    void performApiFetch() {
        std::string symbolStr = currentSymbol.toStdString();
        std::string url = "https://finnhub.io/api/v1/quote?symbol=" + symbolStr + "&token=" + api_key;
        
        cpr::Response r = cpr::Get(cpr::Url{url});

        if (r.status_code == 200) {
            try {
                json data = json::parse(r.text);
                
                if (data.contains("c") && data.contains("pc") && data["c"] != 0) {
                    double current_price = data["c"];
                    double previous_close = data["pc"];
                    double percent_change = ((current_price - previous_close) / previous_close) * 100.0;
                    
                    updateDashboard(current_price, percent_change);
                    stackedWidget->setCurrentIndex(2); // Success -> Show Dashboard
                } else {
                    showError("Invalid symbol or no data returned.");
                }
            } catch (const json::parse_error& e) {
                showError("Failed to parse API response.");
            }
        } else {
            showError(QString("HTTP Error: %1").arg(r.status_code));
        }
    }

    void updateDashboard(double current_price, double percent_change) {
        
        detailsHeaderLabel->setText(currentSymbol + " Overview");

        QString trend = (percent_change > 0) ? "UP ↑" : ((percent_change < 0) ? "DOWN ↓" : "FLAT -");
        QString color = (percent_change > 0) ? "#28a745" : "#dc3545"; // Green or Red
        QString detailsText = QString(
            "Current Price: <b>$%1</b> &nbsp;&nbsp;|&nbsp;&nbsp; "
            "Day Change: <span style='color:%2;'><b>%3% %4</b></span>"
        ).arg(current_price, 0, 'f', 2).arg(color).arg(percent_change, 0, 'f', 2).arg(trend);

        detailsPriceLabel->setText(detailsText);

        // Update History Sidebar
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
        historySidebar->append(QString("<b>[%1] %2</b>: $%3 (%4%)")
            .arg(timestamp).arg(currentSymbol)
            .arg(current_price, 0, 'f', 2).arg(percent_change, 0, 'f', 2));

        sessionPrices[currentSymbol] = current_price;
        updatePieChart();
        lineChart->removeAllSeries(); 
        QLineSeries* series = new QLineSeries();
        
        double start_price = current_price * 0.95; 
        for(int i = 0; i < 6; i++) {
            double random_val = QRandomGenerator::global()->generateDouble(); 
            double random_modifier = (random_val * 0.04) - 0.02; 
            start_price += (start_price * random_modifier);
            series->append(i, start_price);
        }
        series->append(6, current_price); // Day 6 live

        lineChart->addSeries(series);
        lineChart->createDefaultAxes();
    }

    void updatePieChart() {
      pieSeries->clear();
      pieChart->setTitle("Price Comparison (Share Value)");

      for (auto const& [symbol, price] : sessionPrices) {
        QString label = QString("%1 ($%2)").arg(symbol).arg(price, 0, 'f', 2);

        QPieSlice* slice = pieSeries->append(label, price);

        slice->setLabelVisible(true);
        slice->setLabelPosition(QPieSlice::LabelOutside); 

        slice->setLabelColor(Qt::black);
      }
    }

    void showError(const QString& msg) {
        loadingLabel->setText("Error: " + msg + "\n\nPress back and try again.");
        QTimer::singleShot(2500, [this]() {
            stackedWidget->setCurrentIndex(0); // Return home after showing error
        });
    }
};

int main(int argc, char *argv[]) {
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    try { dotenv::init(); } catch (...) {}

    const char* env_p = std::getenv("API_KEY");
    if (!env_p) {
        std::cerr << "Error: API_KEY not found in environment (.env file)." << std::endl;
        return 1;
    }

    QApplication app(argc, argv);

    StockTrackerApp mainWindow(env_p);
    mainWindow.show();

    return app.exec();
}

#include "main.moc" // Include required if writing Q_OBJECT in main.cpp (See note below)
