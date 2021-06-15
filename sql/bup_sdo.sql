-- the following 5 lines will throw errors if the tables already exist
create table bup_sdo_geom_metadata as select * from user_sdo_geom_metadata where rownum < 0;
create table bup_sdo_cached_maps as select * from user_sdo_cached_maps where rownum < 0;
create table bup_sdo_maps as select * from user_sdo_maps where rownum < 0;
create table bup_sdo_themes as select * from user_sdo_themes where rownum < 0;
create table bup_sdo_styles as select * from user_sdo_styles where rownum < 0;

delete from bup_sdo_geom_metadata ;
delete from bup_sdo_maps ;
delete from bup_sdo_themes ;
delete from bup_sdo_styles ;
delete from bup_sdo_cached_maps ;

insert into bup_sdo_geom_metadata select * from user_sdo_geom_metadata;
insert into bup_sdo_maps select * from user_sdo_maps;
insert into bup_sdo_themes select * from user_sdo_themes;
insert into bup_sdo_styles select * from user_sdo_styles;
insert into bup_sdo_cached_maps select * from user_sdo_cached_maps;
