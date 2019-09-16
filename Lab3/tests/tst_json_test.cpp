#include <QtTest>

#include "../qt_server/Server.hpp"
#include "../qt_server/Input.hpp"
#include "../qt_server/Output.hpp"

class json_test : public QObject
{
		Q_OBJECT

	public:
		json_test();
		~json_test();

	private slots:
		void input_test();
		void solver_test();
		void output_test();
		void ping_test();
		void server_test();
};

json_test::json_test() {}
json_test::~json_test() {}

void json_test::input_test()
{
	Input input;
	input.fromJson("{\"K\":10,\"Sums\":[1.01,2.02],\"Muls\":[1,4]}");
	QCOMPARE(input.k, 10);
	QCOMPARE(input.sums, (QVector<double>{1.01, 2.02}));
	QCOMPARE(input.muls, (QVector<int>{1, 4}));
}


void json_test::solver_test()
{
	Input input;
	input.fromJson("{\"K\":10,\"Sums\":[1.01,2.02],\"Muls\":[1,4]}");
	Output output;
	output.solve(input);
	QCOMPARE(output.sumResult, 30.30);
	QCOMPARE(output.mulResult, 4);
	QCOMPARE(output.sorted, (QVector<double>{1, 1.01, 2.02, 4}));
}


void json_test::output_test()
{
	Input input;
	input.fromJson("{\"K\":10,\"Sums\":[1.01,2.02],\"Muls\":[1,4]}");
	Output output;
	output.solve(input);
	std::string result;
	output.toJson(result);
	QCOMPARE(result, "{\"SumResult\":30.30,\"MulResult\":4,\"SortedInputs\":[1.0,1.01,2.02,4.0]}");
}

void json_test::ping_test()
{
	// Start the server
	;
}

void json_test::server_test()
{
	// Start the server
	;
}

QTEST_APPLESS_MAIN(json_test)

#include "tst_json_test.moc"
