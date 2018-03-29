CREATE TABLE T_GATHERINFO(
ID INTEGER PRIMARY KEY autoincrement,
AREA_ID 	VARCHAR(10),
CHNL_NO 	VARCHAR(10),
I_E_TYPE 	VARCHAR(2),
SEQ_NO 		VARCHAR(20),
CHNL_TYPE 	VARCHAR(2),
DR_IC_NO 	VARCHAR(32),
IC_DR_CUSTOMS_NO 	VARCHAR(32),
IC_CO_CUSTOMS_NO 	VARCHAR(32),
IC_BILL_NO 			VARCHAR(32),
IC_GROSS_WT 		VARCHAR(32),
IC_VE_CUSTOMS_NO 	VARCHAR(32),
IC_VE_NAME 			VARCHAR(32),
IC_CONTA_ID 		VARCHAR(32),
IC_ESEAL_ID 		VARCHAR(32),
GROSS_WT 			VARCHAR(32),
VE_NAME 			VARCHAR(32),
VE_NAME_OLD 			VARCHAR(32),
CAR_EC_NO 			VARCHAR(120),
CAR_EC_NO2 			VARCHAR(120),
VE_CUSTOMS_NO 		VARCHAR(32),
VE_WT 				VARCHAR(32),
CONTA_NUM 			VARCHAR(2),
CONTA_RECO 			VARCHAR(2),
CONTA_ID_F 			VARCHAR(32),
CONTA_ID_B 			VARCHAR(32),

CONTA_ID_F_OLD 			VARCHAR(32),
CONTA_ID_B_OLD 			VARCHAR(32),

CONTA_MODEL_F 		VARCHAR(32),
CONTA_MODEL_B 		VARCHAR(32),
ESEAL_ID 			VARCHAR(32),
SEAL_KEY 			VARCHAR(32),
BARCODE 			VARCHAR(32),
GATHER_TIME			DATETIME,
IS_REGATHER			VARCHAR(2),
PASS_MODE			VARCHAR(2),
IC_EX_DATA 			VARCHAR(255),
IC_BUSS_TYPE		VARCHAR(10)
);



