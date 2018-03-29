#ifndef IDDETECT_H_
#define IDDETECT_H_


//ÏäºÅËùÔÚÇøÓò½
typedef struct Region
{
	int x;
	int y;
	int width;
	int height;
	Region()
	{
		x = 0;
		y = 0;
		width = 0;
		height = 0;
	}
}Region;
//ºìÉ«¡¢»ÆÉ«¡¢À¶É«¡¢°×É«¡¢»ÒÉ«¡¢ÂÌÉ«¡¢ÆäËûÑÕÉ«
enum Color {red, yellow, blue, white, gray, green, other};
//ÏäºÅÅÅÁĞ·½Ê½£¬ºáÅÅ£¨Ò»ÅÅ¡¢¶şÅÅ¡¢ÈıÅÅ¡¢ËÄÅÅ£©£¬ÊúÅÅ£¨Ò»Êú¡¢¶şÊú¡¢ÈıÊú£©
enum AlignType {H, T};//H:horizontal ºáÅÅ  T£ºtandem ×İÅÅ
typedef struct Align
{
	AlignType Atype;//H:ºáÅÅ
	int count;//ÅÅÊı
	Align()
	{
		Atype = H;
		count = 0;
	}
}Align;
typedef struct ContainerID
{
	unsigned char ID[12];//4Î»×ÖÄ¸+6Î»ÏäºÅ+1Î»Ğ£ÑéÂë+ÖÕÖ¹·û
	unsigned char Type[5];//ÀàĞÍ
	Region IDreg;////ÏäºÅÇøÓò
	Region Typereg;//ÏäĞÍÇøÓò
	Color color;//ÑÕÉ«
	Align ali;//ÅÅÁĞ·½Ê½
	float accuracy;//Ê¶±ğ¾«¶È
}ContainerID;

extern "C" int  PathReadCode(char *ImagePath,ContainerID *code);//½Ó¿Ú1
extern "C" int  CReadCode(unsigned char *Image, int width, int height, ContainerID *code);//½Ó¿Ú2
extern "C" int  BReadCode(unsigned char * buff, int width, int height, ContainerID *code);//½Ó¿Ú3
#endif