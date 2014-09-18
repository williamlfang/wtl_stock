#include <vector>
#include "gtest/gtest.h"
#include "global.h"
#include "db_quote.h"

//决定：
//1.单元测试，不需要实际数据，测试理论值即可
//2.采用wind提出的涨跌幅前复权，换言之，保证登记日和复权前一天的涨跌幅正确，保证当日价格和收盘价的偏离正确。
//3.先测试3中情形下的前复权问题


//测试数据，设置10个相连的日期
std::vector<Quote>   quotes =
{
	{1409011200},
	{1409097600},
	{1409184000},
	{1409270400},
	{1409529600},
	{1409616000},
	{1409702400},
	{1409788800},
	{1409875200},
	{1410220800}
};
std::vector<ExRight> exrights;
//测试向前复权算法
//模拟10条数据,3条权息数据
//为权息数据，计算复权因子和时间段
//复权数据使用实际的数据..测试数据来自实际数据中的几个关键点即前面一条。
//巨化股份，包括：1、分红；2、送股；3、配股；4、送股加分红
//2006


//向前复权：计算股权登记日
//搜索日线，如找到日期，改日期为股权登记日
//如未找到，则插入位置(第一个大于该日期的记录)+1，也就是第一个大于该日期的，为登记日
//登记日序号-1为前复权应计算的位置。
TEST(AdjustPrice, GetExOrder)
{
	DbQuote dbQuote;

	//查找第二个：
	int rc = dbQuote.GetExOrder(quotes, 1409097600);
	EXPECT_EQ(1, rc);

	//查找不存在，但比第二个小的
	rc = dbQuote.GetExOrder(quotes, 1409097599);
	EXPECT_EQ(1, rc);

	//越界：查找比第一个小的
	rc = dbQuote.GetExOrder(quotes, 1409011199);
	EXPECT_EQ(0, rc);

	//越界：查找比最后一个大的
	rc = dbQuote.GetExOrder(quotes, 1410220801);
	EXPECT_EQ(10, rc); //返回最后一个序号+1，新的插入位置

}
//航天动力 600343
//20140423为股权登记日，前一天开始复权...即小于股权登记日，且不等于。
//前复权：复权后价格＝[(复权前价格 - 现金红利)＋配(新)股价格×流通股份变动比例]÷(1＋流通股份变动比例)
//后复权：复权后价格＝复权前价格×(1＋流通股份变动比例) - 配(新)股价格×流通股份变动比例＋现金红利
TEST(AdjustPrice, CaculateFactor) //计算复权因子
{

}

TEST(AdjustPrice, Adjust) //测试前复权结果，包括四个价格、成交量
{

}

//计算流通股本(按照最新流通股和送配股情形)
//流通股本保存在权息数据里
TEST(AdjustPrice,TotalShares) 
{

}