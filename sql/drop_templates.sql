spool drop_templates.log
drop table TEMPLATE_DLM_PRIME2 purge;
drop table TEMPLATE_DLM_DLM purge;
drop table TEMPLATE_DLM_WAY purge;
drop table TEMPLATE_DLM_WATER purge;
drop table TEMPLATE_DLM_WATER_SINGLE purge;
drop table TEMPLATE_DLM_ARTIFICIAL purge;
drop table TEMPLATE_DLM_BUILDING purge;
drop table TEMPLATE_DLM_EXPOSED purge;
drop table TEMPLATE_DLM_RAIL_SEGMENT purge;
drop table TEMPLATE_DLM_SITE purge;
drop table TEMPLATE_DLM_STRUCTURE purge;
drop table TEMPLATE_DLM_VEGETATION purge;
drop table TEMPLATE_DLM_DIVISION purge;
drop table TEMPLATE_DLM_LOCALE purge;
delete from user_sdo_geom_metadata where table_name like 'TEMPLATE_2D' or table_name like 'TEMPLATE_3D';
commit;
spool off
