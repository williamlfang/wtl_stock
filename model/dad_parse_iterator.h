#ifndef dad_parse_iterator_h
#define dad_parse_iterator_h
#include "parse_iterator.h"

//Dad文件的证卷代码、名称对照表结构
struct id_of_dad
{
	char id[16]; //16位仅为保留偏移
	char title[16];
};

//Dad文件的行情记录结构
struct quote_of_dad
{
	long quoteTime; //使用_mktime32转为__time32t存储，使用_localtime32读取
	float open, high, low, close;
	float volume, amount;
};

//Dad文件的识别结构
struct parse_of_dad
{
	id_of_dad *idOfDad;
	quote_of_dad *quoteOfDad;
};

//用于识别dad,由此提供parse_of_dad结构
class dad_parse_iterator :public parse_iterator<parse_of_dad>
{
public:
	using self_type = dad_parse_iterator;
	dad_parse_iterator() = default;
	dad_parse_iterator(char * _begin, char * _end, parse_of_dad _value):
		parse_iterator<parse_of_dad>(_begin, _end, _value)
	{

	}

	self_type& operator++()
	{
		//如果已经到end，则++操作什么都不做
		if (value_.quoteOfDad == nullptr)
			return *this;
		value_.quoteOfDad=(quote_of_dad *)((char *)value_.quoteOfDad + 32);
		if (*(long *)(value_.quoteOfDad) == 0xffffffff) //如果时间为0xffffffff，表示此时进入下一只股票
		{
			//文件的最后，0xffffffff开始，然后全0.换言之，下一只股票，但id为空，表示文件结束。
			//最后  FF FF FF FF 00 00 00 00  00 00 00 00 00 00 00 00   
			//      00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
			if ((*((char *)value_.quoteOfDad + 4) == 0x00))  //尾部是0xffffffff四个字节后一堆0.
			{
				value_.idOfDad = nullptr; //这样是否有内存泄漏？
				value_.quoteOfDad = nullptr;
			}
			else  //此时确定为下一只股票
			{
				value_.idOfDad = (id_of_dad *)((char *)value_.quoteOfDad + 4);  //修改id
				value_.quoteOfDad = (quote_of_dad *)((char *)value_.quoteOfDad + 32);//移动32位
			}
		}
		return *this;
	};

	//覆盖，基类不能是virtual
	bool  operator==(const self_type& _next) 
	{
		return (value_.quoteOfDad == _next.value_.quoteOfDad);
	};

	bool  operator!=(const self_type& _next)
	{
		return (value_.quoteOfDad != _next.value_.quoteOfDad);
	};
};


#endif