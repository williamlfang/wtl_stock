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
	{1409011200,21.58f,21.79f,21.90f,21.21f,87393.0f,187047984.0f}, //20120518，除权前一日
	{1409097600,12.88f,12.90f,12.57f,12.68f,50629.0f,64518388.0f},  //20120512,除权日
	{ 1409184000, 7.81f, 7.94f, 7.72f, 7.93f, 111151.0f, 87543768.000f},//20130517,除权前一日
	{ 1409270400, 7.88f, 8.13f, 7.72f, 8.11f, 136405.0f, 108198920.000f},//20130520,除权日
	{ 1409529600, 6.4f, 6.52f, 6.32f, 6.33f, 178206, 113996256.000f},//20131213除权前一日，之后停牌10天
	{ 1409616000, 5.44f, 5.61f, 5.34f, 5.48f, 126405.0f, 69211328.000f},//20131224除权日
	{1409702400,5.13f,5.17f,5.10f,5.16f,121149.0f,62278860.000f},//20140616除权前2日
	{1409788800,5.13f,5.16f,5.10f,5.16f,137956.0f,70828760.0f}, //20140617 除权前一日
	{1409875200,4.97f,4.98f,4.80f,4.83f,156783.0f,76399296.000f}, //20140618 除权日
	{1410220800,4.82f,4.92f,4.67f,4.69f,135345.0f,64897560.0f},//20140619 除权后一日
};
//巨化股份前四条权息数据
//注意时间倒叙排列，最新在前
std::vector<ExRight> exrights = {
	{ 1409875200, 0.0f, 0.0f, 0.0f, 0.2f },
	{ 1409616000, 0.0f,0.3f, 4.23f, 0.0f },
	{ 1409270400, 0.0f, 0.0f, 0.0f, 0.15f},
	{ 1409097600,0.6f,0.0f,0.0f,0.5f},
};
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

	//将所有的序号找出来
	dbQuote.GetExOrder(quotes, exrights);
	EXPECT_EQ(8, exrights[0].Start);
	EXPECT_EQ(5, exrights[1].Start);
	EXPECT_EQ(3, exrights[2].Start);
	EXPECT_EQ(1, exrights[3].Start);


}
//航天动力 600343
//20140423为股权登记日，前一天开始复权...即小于股权登记日，且不等于。
//前复权：复权后价格＝[(复权前价格 - 现金红利)＋配(新)股价格×流通股份变动比例]÷(1＋流通股份变动比例)
//后复权：复权后价格＝复权前价格×(1＋流通股份变动比例) - 配(新)股价格×流通股份变动比例＋现金红利
TEST(AdjustPrice, CaculateFactor) //计算复权因子
{
	DbQuote dbQuote;

	//先计算顺序
	dbQuote.GetExOrder(quotes, exrights);

	//计算第一个复权因子
	float factor=dbQuote.CaculateFactor(quotes, exrights[0]);
	//EXPECT_FLOAT_EQ(0.96124035f,factor);
	EXPECT_FLOAT_EQ(4.96f, quotes[exrights[0].Start - 1].Close/factor);
	//6月17日，大智慧实际5.16，前复权计算的价格为4.96,通达信相同。

	factor = factor*dbQuote.CaculateFactor(quotes, exrights[1]); //计算倒数第二次复权因子
	EXPECT_FLOAT_EQ(5.6188192f, quotes[exrights[1].Start - 1].Close/factor);
	//通达信前复权值为5.65,差0.03

	//误差有0.03，似乎是缩减了中间数据造成的？
	//若通达信使用递归前复权？
	dbQuote.CaculateFactor(quotes, exrights);
	EXPECT_FLOAT_EQ(4.96f, quotes[exrights[0].Start - 1].Close/exrights[0].Factor); //4.96通达信和大智慧4.96
	EXPECT_FLOAT_EQ(5.6188192f, quotes[exrights[1].Start - 1].Close/exrights[1].Factor); //5.62通达信和大智慧5.65

	//误差更大，通达信、大智慧均为6.76
	EXPECT_FLOAT_EQ(6.9059105f, quotes[exrights[2].Start - 1].Close/exrights[2].Factor);//6.91通达信和大智慧6.76



}

TEST(AdjustPrice, Adjust) //测试前复权结果，包括四个价格、成交量
{
	DbQuote dbQuote;
	std::vector<Quote> backup_quote;
	backup_quote.assign(quotes.begin(), quotes.end());
	dbQuote.AdjustPrice(backup_quote,exrights);

	float rate_for_old = (quotes[7].Close - quotes[6].Close) / quotes[6].Close;
	float rate_for_adjust = (backup_quote[7].Close - backup_quote[6].Close) / backup_quote[6].Close;
	EXPECT_FLOAT_EQ(rate_for_adjust, rate_for_old);

}

//计算流通股本(按照最新流通股和送配股情形)
//流通股本保存在权息数据里
TEST(AdjustPrice,TotalShares) 
{

}