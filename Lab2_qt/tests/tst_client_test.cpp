#include <QtTest>

#include <stdio.h>
#include <time.h>
#include <random>
#include <QtHttpServer>
#include <QCoreApplication>
#include <QDebug>

class json_test : public QObject
{
	Q_OBJECT
	private:
		FILE* cfd;
		int port;
		QHttpServer serv;
		QByteArray answer;

	public:
		json_test();
		~json_test();

	private slots:
		void initTestCase();

		void solver_test();

		void cleanupTestCase();
};

json_test::json_test() {}
json_test::~json_test() {}

void json_test::initTestCase()
{
	cfd = popen("../Lab2/Lab2", "w");
	srand(time(0));
	//port = rand() % 10000 + 10000;
	port = 20000;
	serv.listen(QHostAddress::Any, port);
	serv.route("Ping", [](){return QHttpServerResponse::StatusCode::Ok;});
	serv.route("GetInputData", [](){return "{\"K\":10,\"Sums\":[1.01,2.02],\"Muls\":[1,4]}";});
		//"{\"SumResult\":30.30,\"MulResult\":4,\"SortedInputs\":[1.0,1.01,2.02,4.0]}"
	serv.route("WriteAnswer", [&](const QHttpServerRequest &request)
	{
		answer = request.body();
		qDebug() << "Received an answer: " << answer;
		return QHttpServerResponse::StatusCode::Ok;
	});
	fprintf(cfd, "%d\n", port);
}

void json_test::solver_test()
{
	while(answer.isEmpty())
	{
		QCoreApplication::processEvents();
	}
	QCOMPARE(answer, "{\"SumResult\":30.30,\"MulResult\":4,\"SortedInputs\":[1.0,1.01,2.02,4.0]}");
}

void json_test::cleanupTestCase()
{
	pclose(cfd);
}

QTEST_APPLESS_MAIN(json_test)

#include "tst_client_test.moc"
