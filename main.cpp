#include <QApplication>
#include <QWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QString>
#include <string>
#include <QDoubleValidator>
#include <iostream>
#include <cctype>
#include <stdexcept>
#include <string>
#include <cmath>
#include <QPainter>
#include <QPushButton>

using namespace std;

class Parser {
    string input;
    size_t pos;
    double xValue;

    char peek() const {
        return pos < input.size() ? input[pos] : '\0';
    }

    char get() {
        return input[pos++];
    }

    double number() {
        string numStr;
        while (isdigit(peek()) || peek() == '.') {
            numStr += get();
        }
        if (numStr.empty()) {
            throw runtime_error("Expected a number");
        }
        return stod(numStr);
    }

    double factor() {
        if (isalpha(peek())) {
            string func;
            while (isalpha(peek())) {
                func += get();
            }
            if (func == "x") {
                return xValue;
            } else if (func == "sin") {
                return sin(parseArgument());
            } else if (func == "cos") {
                return cos(parseArgument());
            } else if (func == "tg") {
                return tan(parseArgument());
            } else if (func == "ctg") {
                return 1.0 / tan(parseArgument());
            } else {
                throw runtime_error("Unknown function: " + func);
            }
        }
        if (peek() == '(') {
            get();
            double result = expression();
            if (get() != ')') {
                throw runtime_error("Expected ')'");
            }
            return result;
        }
        return number();
    }

    double parseArgument() {
        if (peek() == '(') {
            get();
            double result = expression();
            if (get() != ')') {
                throw runtime_error("Expected ')'");
            }
            return result;
        }
        throw runtime_error("Expected '(' after function name");
    }

    double power() {
        double result = factor();
        while (peek() == '^') {
            get();
            result = pow(result, factor());
        }
        return result;
    }

    double term() {
        double result = power();
        while (true) {
            if (peek() == '*') {
                get();
                result *= power();
            } else if (peek() == '/') {
                get();
                double divisor = power();
                if (divisor == 0) {
                    throw runtime_error("Division by zero");
                }
                result /= divisor;
            } else {
                break;
            }
        }
        return result;
    }

    double expression() {
        double result = term();
        while (true) {
            if (peek() == '+') {
                get();
                result += term();
            } else if (peek() == '-') {
                get();
                result -= term();
            } else {
                break;
            }
        }
        return result;
    }

public:
    void setX(double value) {
        xValue = value;
    }

    double parse(const string& expr) {
        input = expr;
        pos = 0;
        double result = expression();
        if (pos != input.size()) {
            throw runtime_error("Unexpected characters at end of input");
        }
        return result;
    }
};

class GraphWidget : public QWidget {
    vector<int> xtodraw;
    vector<int> ytodraw;

public:
    GraphWidget(const vector<int>& x, const vector<int>& y, QWidget* parent = nullptr)
        : QWidget(parent), xtodraw(x), ytodraw(y) {}

protected:
    void paintEvent(QPaintEvent* event) override {
        QPainter painter(this);
        QPen pen(Qt::red);
        pen.setWidth(2);
        painter.setPen(pen);

        for (size_t i = 0; i < xtodraw.size(); ++i) {
            painter.drawPoint(xtodraw[i], ytodraw[i]);
        }
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    QVBoxLayout* layout = new QVBoxLayout(&window);

    QLineEdit* FUNC = new QLineEdit(&window);
    QLineEdit* LEFTBORD = new QLineEdit(&window);
    QLineEdit* RIGHTBORD = new QLineEdit(&window);
    QPushButton* drawButton = new QPushButton("Намалювати", &window);

    FUNC->setPlaceholderText("функція (наприклад: sin(x))");
    LEFTBORD->setPlaceholderText("ліва межа");
    RIGHTBORD->setPlaceholderText("права межа");

    QDoubleValidator* dbValidator = new QDoubleValidator(&window);
    dbValidator->setRange(-1000.0, 1000.0, 4);

    LEFTBORD->setValidator(dbValidator);
    RIGHTBORD->setValidator(dbValidator);

    layout->addWidget(FUNC);
    layout->addWidget(LEFTBORD);
    layout->addWidget(RIGHTBORD);
    layout->addWidget(drawButton);

    window.setLayout(layout);
    window.show();

    QObject::connect(drawButton, &QPushButton::clicked, [&]() {
        string func = FUNC->text().toStdString();
        double LB = LEFTBORD->text().toDouble();
        double RB = RIGHTBORD->text().toDouble();

        int acc = 100000;
        double step = (RB - LB) / acc;
        vector<double> xpts(acc);
        vector<double> ypts(acc);
        double ymax = 0;

        for (int i = 0; i < acc; ++i) {
            xpts[i] = LB + step * i;
            try {
                Parser parser;
                parser.setX(xpts[i]);
                ypts[i] = parser.parse(func);
                if (fabs(ypts[i]) > ymax) ymax = fabs(ypts[i]);
            } catch (const exception& e) {
                cerr << "Error: " << e.what() << endl;
                return;
            }
        }

        double convx = 1200.0 / (RB - LB);
        double convy = 600.0 / ymax;

        vector<int> xtodraw(acc);
        vector<int> ytodraw(acc);

        for (int i = 0; i < acc; ++i) {
            xtodraw[i] = static_cast<int>((xpts[i] - LB) * convx);
            ytodraw[i] = static_cast<int>(300 - ypts[i] * convy);
        }

        GraphWidget* graph = new GraphWidget(xtodraw, ytodraw);
        graph->resize(1200, 600);
        graph->show();
    });

    return app.exec();
}
