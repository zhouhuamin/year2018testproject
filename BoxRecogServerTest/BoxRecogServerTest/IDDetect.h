#ifndef IDDETECT_H_
#define IDDETECT_H_


//
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
//��ɫ����ɫ����ɫ����ɫ����ɫ����ɫ��������ɫ
enum Color {red, yellow, blue, white, gray, green, other};
//������з�ʽ�����ţ�һ�š����š����š����ţ������ţ�һ����������������
enum AlignType {H, T};//H:horizontal ����  T��tandem ����
typedef struct Align
{
	AlignType Atype;//H:����
	int count;//����
	Align()
	{
		Atype = H;
		count = 0;
	}
}Align;
typedef struct ContainerID
{
	unsigned char ID[12];
	unsigned char Type[5];
	Region IDreg;
	Region Typereg;
	Color color;
	Align ali;
	float accuracy;
}ContainerID;

extern "C" int  PathReadCode(char *ImagePath,ContainerID *code);//�ӿ�1
extern "C" int  CReadCode(unsigned char *Image, int width, int height, ContainerID *code);//�ӿ�2
extern "C" int  BReadCode(unsigned char * buff, int width, int height, ContainerID *code);//�ӿ�3
#endif