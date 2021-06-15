spool create_dlm_tables.log

create table dlm_0_artificial as select * from template_dlm_artificial;
alter table dlm_0_artificial add guid varchar2(38);
create table dlm_0_artificial_prime2 as select * from template_dlm_prime2;
create sequence dlm_0_artificial_seq;

create table dlm_0_building as select * from template_dlm_building;
alter table dlm_0_building add guid varchar2(38);
create table dlm_0_building_prime2 as select * from template_dlm_prime2;
create sequence dlm_0_building_seq;

create table dlm_0_division as select * from template_dlm_division;
alter table dlm_0_division add guid varchar2(38);
create table dlm_0_division_prime2 as select * from template_dlm_prime2;
create sequence dlm_0_division_seq;

create table dlm_0_exposed as select * from template_dlm_exposed;
alter table dlm_0_exposed add guid varchar2(38);
create table dlm_0_exposed_prime2 as select * from template_dlm_prime2;
create sequence dlm_0_exposed_seq;

create table dlm_0_locale as select * from template_dlm_locale;
alter table dlm_0_locale add guid varchar2(38);
create table dlm_0_locale_prime2 as select * from template_dlm_prime2;
create sequence dlm_0_locale_seq;

create table dlm_0_rail_segment as select * from template_dlm_rail_segment;
alter table dlm_0_rail_segment add guid varchar2(38);
create table dlm_0_rail_segment_prime2 as select * from template_dlm_prime2;
create sequence dlm_0_rail_segment_seq;

create table dlm_0_site as select * from template_dlm_site;
alter table dlm_0_site add guid varchar2(38);
create table dlm_0_site_prime2 as select * from template_dlm_prime2;
create sequence dlm_0_site_seq;

create table dlm_0_structure as select * from template_dlm_structure;
alter table dlm_0_structure add guid varchar2(38);
create table dlm_0_structure_prime2 as select * from template_dlm_prime2;
create sequence dlm_0_structure_seq;

create table dlm_0_vegetation as select * from template_dlm_vegetation;
alter table dlm_0_vegetation add guid varchar2(38);
create table dlm_0_vegetation_prime2 as select * from template_dlm_prime2;
create sequence dlm_0_vegetation_seq;

create table dlm_0_water as select * from template_dlm_water;
alter table dlm_0_water add guid varchar2(38);
create table dlm_0_water_prime2 as select * from template_dlm_prime2;
create sequence dlm_0_water_seq;

create table dlm_0_water_single as select * from template_dlm_water_single;
alter table dlm_0_water_single add guid varchar2(38);
create table dlm_0_water_single_prime2 as select * from template_dlm_prime2;
create sequence dlm_0_water_single_seq;

create table dlm_0_way as select * from template_dlm_way;
alter table dlm_0_way add guid varchar2(38);
create table dlm_0_way_prime2 as select * from template_dlm_prime2;
create sequence dlm_0_way_seq;

--------------------------------------------------------------------------
create table dlm_1_artificial as select * from template_dlm_artificial;
create table dlm_1_artificial_prime2 as select * from template_dlm_prime2;
create sequence dlm_1_artificial_seq;

create table dlm_1_building as select * from template_dlm_building;
create table dlm_1_building_prime2 as select * from template_dlm_prime2;
create sequence dlm_1_building_seq;

create table dlm_1_exposed as select * from template_dlm_exposed;
create table dlm_1_exposed_prime2 as select * from template_dlm_prime2;
create sequence dlm_1_exposed_seq;

create table dlm_1_rail_segment as select * from template_dlm_rail_segment;
create table dlm_1_rail_segment_prime2 as select * from template_dlm_prime2;
create sequence dlm_1_rail_segment_seq;

create table dlm_1_site as select * from template_dlm_site;
create table dlm_1_site_prime2 as select * from template_dlm_prime2;
create sequence dlm_1_site_seq;

create table dlm_1_structure as select * from template_dlm_structure;
create table dlm_1_structure_prime2 as select * from template_dlm_prime2;
create sequence dlm_1_structure_seq;

create table dlm_1_vegetation as select * from template_dlm_vegetation;
create table dlm_1_vegetation_prime2 as select * from template_dlm_prime2;
create sequence dlm_1_vegetation_seq;

create table dlm_1_water as select * from template_dlm_water;
create table dlm_1_water_prime2 as select * from template_dlm_prime2;
create sequence dlm_1_water_seq;

create table dlm_1_water_single as select * from template_dlm_water_single;
create table dlm_1_water_single_prime2 as select * from template_dlm_prime2;
create sequence dlm_1_water_single_seq;

create table dlm_1_way as select * from template_dlm_way;
create table dlm_1_way_prime2 as select * from template_dlm_prime2;
create sequence dlm_1_way_seq;

--------------------------------------------------------------------------
create table dlm_3_artificial as select * from template_dlm_artificial;
create table dlm_3_artificial_prime2 as select * from template_dlm_prime2;
create sequence dlm_3_artificial_seq;

create table dlm_3_building as select * from template_dlm_building;
create table dlm_3_building_prime2 as select * from template_dlm_prime2;
create sequence dlm_3_building_seq;

create table dlm_3_exposed as select * from template_dlm_exposed;
create table dlm_3_exposed_prime2 as select * from template_dlm_prime2;
create sequence dlm_3_exposed_seq;

create table dlm_3_rail_segment as select * from template_dlm_rail_segment;
create table dlm_3_rail_segment_prime2 as select * from template_dlm_prime2;
create sequence dlm_3_rail_segment_seq;

create table dlm_3_site as select * from template_dlm_site;
create table dlm_3_site_prime2 as select * from template_dlm_prime2;
create sequence dlm_3_site_seq;

create table dlm_3_structure as select * from template_dlm_structure;
create table dlm_3_structure_prime2 as select * from template_dlm_prime2;
create sequence dlm_3_structure_seq;

create table dlm_3_vegetation as select * from template_dlm_vegetation;
create table dlm_3_vegetation_prime2 as select * from template_dlm_prime2;
create sequence dlm_3_vegetation_seq;

create table dlm_3_water as select * from template_dlm_water;
create table dlm_3_water_prime2 as select * from template_dlm_prime2;
create sequence dlm_3_water_seq;

create table dlm_3_water_single as select * from template_dlm_water_single;
create table dlm_3_water_single_prime2 as select * from template_dlm_prime2;
create sequence dlm_3_water_single_seq;

create table dlm_3_way as select * from template_dlm_way;
create table dlm_3_way_prime2 as select * from template_dlm_prime2;
create sequence dlm_3_way_seq;

--------------------------------------------------------------------------
create table dlm_6_artificial as select * from template_dlm_artificial;
create table dlm_6_artificial_prime2 as select * from template_dlm_prime2;
create sequence dlm_6_artificial_seq;

create table dlm_6_building as select * from template_dlm_building;
create table dlm_6_building_prime2 as select * from template_dlm_prime2;
create sequence dlm_6_building_seq;

create table dlm_6_exposed as select * from template_dlm_exposed;
create table dlm_6_exposed_prime2 as select * from template_dlm_prime2;
create sequence dlm_6_exposed_seq;

create table dlm_6_rail_segment as select * from template_dlm_rail_segment;
create table dlm_6_rail_segment_prime2 as select * from template_dlm_prime2;
create sequence dlm_6_rail_segment_seq;

create table dlm_6_site as select * from template_dlm_site;
create table dlm_6_site_prime2 as select * from template_dlm_prime2;
create sequence dlm_6_site_seq;

create table dlm_6_structure as select * from template_dlm_structure;
create table dlm_6_structure_prime2 as select * from template_dlm_prime2;
create sequence dlm_6_structure_seq;

create table dlm_6_vegetation as select * from template_dlm_vegetation;
create table dlm_6_vegetation_prime2 as select * from template_dlm_prime2;
create sequence dlm_6_vegetation_seq;

create table dlm_6_water as select * from template_dlm_water;
create table dlm_6_water_prime2 as select * from template_dlm_prime2;
create sequence dlm_6_water_seq;

create table dlm_6_water_single as select * from template_dlm_water_single;
create table dlm_6_water_single_prime2 as select * from template_dlm_prime2;
create sequence dlm_6_water_single_seq;

create table dlm_6_way as select * from template_dlm_way;
create table dlm_6_way_prime2 as select * from template_dlm_prime2;
create sequence dlm_6_way_seq;

--------------------------------------------------------------------------
create table dlm_15_artificial as select * from template_dlm_artificial;
create table dlm_15_artificial_prime2 as select * from template_dlm_prime2;
create sequence dlm_15_artificial_seq;

create table dlm_15_building as select * from template_dlm_building;
create table dlm_15_building_prime2 as select * from template_dlm_prime2;
create sequence dlm_15_building_seq;

create table dlm_15_exposed as select * from template_dlm_exposed;
create table dlm_15_exposed_prime2 as select * from template_dlm_prime2;
create sequence dlm_15_exposed_seq;

create table dlm_15_rail_segment as select * from template_dlm_rail_segment;
create table dlm_15_rail_segment_prime2 as select * from template_dlm_prime2;
create sequence dlm_15_rail_segment_seq;

create table dlm_15_site as select * from template_dlm_site;
create table dlm_15_site_prime2 as select * from template_dlm_prime2;
create sequence dlm_15_site_seq;

create table dlm_15_structure as select * from template_dlm_structure;
create table dlm_15_structure_prime2 as select * from template_dlm_prime2;
create sequence dlm_15_structure_seq;

create table dlm_15_vegetation as select * from template_dlm_vegetation;
create table dlm_15_vegetation_prime2 as select * from template_dlm_prime2;
create sequence dlm_15_vegetation_seq;

create table dlm_15_water as select * from template_dlm_water;
create table dlm_15_water_prime2 as select * from template_dlm_prime2;
create sequence dlm_15_water_seq;

create table dlm_15_water_single as select * from template_dlm_water_single;
create table dlm_15_water_single_prime2 as select * from template_dlm_prime2;
create sequence dlm_15_water_single_seq;

create table dlm_15_way as select * from template_dlm_way;
create table dlm_15_way_prime2 as select * from template_dlm_prime2;
create sequence dlm_15_way_seq;

spool off
