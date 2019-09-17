#include <QtTest>

#include "../Lab/Input.hpp"
#include "../Lab/Output.hpp"

class json_test : public QObject
{
		Q_OBJECT

	public:
		json_test();
		~json_test();

	private slots:
		void input_json_test();
		void input_json_prop_test();
		void output_json_test();
		void output_json_prop_test();
		void input_xml_test();
		void input_xml_prop_test();
		void output_xml_test();
		void output_xml_prop_test();
		void solver_test();
};

json_test::json_test() {}
json_test::~json_test() {}

void json_test::input_json_test()
{
	Input input;
	input.fromJson("{\"K\":10,\"Sums\":[1.01,2.02],\"Muls\":[1,4]}");
	QCOMPARE(input.k(), 10);
	QCOMPARE(input.sums(), (QVector<double>{1.01, 2.02}));
	QCOMPARE(input.muls(), (QVector<int>{1, 4}));
}

void json_test::input_json_prop_test()
{
	Input input;
	Input::fromJson("{\"K\":10,\"Sums\":[1.01,2.02],\"Muls\":[1,4]}", &input);
	QCOMPARE(input.k(), 10);
	QCOMPARE(input.sums(), (QVector<double>{1.01, 2.02}));
	QCOMPARE(input.muls(), (QVector<int>{1, 4}));
}

void json_test::output_json_test()
{
	Output output;
	output.setSumResult(30.30);
	output.setMulResult(4);
	output.setSorted({1, 1.01, 2.02, 4});
	// NOTE: I used the Qt's standard library JSON which sorts the fields AND does NOT allow to select the presentation for float values, so I had to adapt the test string
	//QCOMPARE(output.toJson(), "{\"SumResult\":30.30,\"MulResult\":4,\"SortedInputs\":[1.0,1.01,2.02,4.0]}");
	QCOMPARE(output.toJson(), "{\"MulResult\":4,\"SortedInputs\":[1,1.01,2.02,4],\"SumResult\":30.3}");

}

void json_test::output_json_prop_test()
{
	Output output;
	output.setSumResult(30.30);
	output.setMulResult(4);
	output.setSorted({1, 1.01, 2.02, 4});
	// NOTE: I used the Qt's standard library JSON which sorts the fields AND does NOT allow to select the presentation for float values, so I had to adapt the test string
	//QCOMPARE(output.toJson(), "{\"SumResult\":30.30,\"MulResult\":4,\"SortedInputs\":[1.0,1.01,2.02,4.0]}");
	QCOMPARE(Output::toJson(&output), "{\"MulResult\":4,\"SortedInputs\":[1,1.01,2.02,4],\"SumResult\":30.3}");

}

void json_test::input_xml_test()
{
	Input input;
	input.fromXml("<Input><K>10</K><Sums><decimal>1.01</decimal><decimal>2.02</decimal></Sums><Muls><int>1</int><int>4</int></Muls></Input>");
	QCOMPARE(input.k(), 10);
	QCOMPARE(input.sums(), (QVector<double>{1.01, 2.02}));
	QCOMPARE(input.muls(), (QVector<int>{1, 4}));
}

void json_test::input_xml_prop_test()
{
	Input input;
	Input::fromXml("<Input><K>10</K><Sums><decimal>1.01</decimal><decimal>2.02</decimal></Sums><Muls><int>1</int><int>4</int></Muls></Input>", &input);
	QCOMPARE(input.k(), 10);
	QCOMPARE(input.sums(), (QVector<double>{1.01, 2.02}));
	QCOMPARE(input.muls(), (QVector<int>{1, 4}));
}

void json_test::output_xml_test()
{
	Output output;
	output.setSumResult(30.30);
	output.setMulResult(4);
	output.setSorted({1, 1.01, 2.02, 4});
	//QCOMPARE(output.toXml(), "<Output><SumResult>30.30</SumResult><MulResult>4</MulResult><SortedInputs><decimal>1</decimal><decimal>1.01</decimal><decimal>2.02</decimal><decimal>4</decimal></SortedInputs></Output>");
	QCOMPARE(output.toXml(), "<Output><SumResult>30.3</SumResult><MulResult>4</MulResult><SortedInputs><decimal>1</decimal><decimal>1.01</decimal><decimal>2.02</decimal><decimal>4</decimal></SortedInputs></Output>");
}

void json_test::output_xml_prop_test()
{
	Output output;
	output.setSumResult(30.30);
	output.setMulResult(4);
	output.setSorted({1, 1.01, 2.02, 4});
	//QCOMPARE(output.toXml(), "<Output><SumResult>30.30</SumResult><MulResult>4</MulResult><SortedInputs><decimal>1</decimal><decimal>1.01</decimal><decimal>2.02</decimal><decimal>4</decimal></SortedInputs></Output>");
	QCOMPARE(Output::toXml(&output), "<Output><MulResult>4</MulResult><SortedInputs><decimal>1</decimal><decimal>1.01</decimal><decimal>2.02</decimal><decimal>4</decimal></SortedInputs><SumResult>30.3</SumResult></Output>");
}

void json_test::solver_test()
{
	Input input;
	input.setK(10);
	input.setSums({1.01, 2.02});
	input.setMuls({1, 4});
	Output output;
	output.solve(input);
	QCOMPARE(output.sumResult(), 30.30);
	QCOMPARE(output.mulResult(), 4);
	QCOMPARE(output.sorted(), (QVector<double>{1, 1.01, 2.02, 4}));
}

QTEST_APPLESS_MAIN(json_test)

#include "tst_serializer_test.moc"
