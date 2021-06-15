spool create_templates.log
insert into user_sdo_geom_metadata values (
'TEMPLATE_3D','GEOM_VALUE',
SDO_DIM_ARRAY(SDO_DIM_ELEMENT('X', 400000, 800000, .0005), SDO_DIM_ELEMENT('Y',500000, 1000000, .0005), SDO_DIM_ELEMENT('Z', -10000,10000, .0005)),
2157
);
insert into user_sdo_geom_metadata values (
'TEMPLATE_2D','GEOM_VALUE',
SDO_DIM_ARRAY(SDO_DIM_ELEMENT('X', 400000, 800000, .0005), SDO_DIM_ELEMENT('Y',500000, 1000000, .0005)),
2157
);
create table template_dlm_prime2 (
dlm_id          number(10),
prime2_guid     varchar2(38)
);
create table template_dlm_dlm (
src_id          number(10),
dlm_id          number(10)
);
create table template_dlm_artificial (
id              number(10),
product         number(10),
scale           number(10),
form_id         number(10),
func_id         number(10),
status          number(10),
primary_name    varchar2(128), -- empty in demo
secondary_name  varchar2(128), -- empty in demo
z_order         number(5),
area            number(12),
perimeter       number(12),
length          number(12),
line_geom_value sdo_geometry,
poly_geom_value sdo_geometry
);
create table template_dlm_building (
id              number(10),
product         number(10),
scale           number(10),
form_id         number(10),
func_id         number(10),
status          number(10),
primary_name    varchar2(128),
secondary_name  varchar2(128), -- empty in demo
building_num    varchar2(128),
way_guid        varchar2(38),
z_order         number(5),
pnt_geom_value  sdo_geometry,
poly_geom_value sdo_geometry
);
create table template_dlm_exposed (
id              number(10),
product         number(10),
scale           number(10),
form_id         number(10),
func_id         number(10),
status          number(10),
primary_name    varchar2(128), -- empty in demo
secondary_name  varchar2(128), -- empty in demo
z_order         number(5),
area            number(12),
perimeter       number(12),
poly_geom_value sdo_geometry
);
create table template_dlm_division (
id              number(10),
product         number(10),
scale           number(10),
form_id         number(10),
func_id         number(10),
status          number(10),
z_order         number(5),
length          number(12),
line_geom_value sdo_geometry
);
create table template_dlm_locale (
id              number(10),
product         number(10),
scale           number(10),
form_id         number(10),
func_id         number(10),
status          number(10),
primary_name    varchar2(128),
secondary_name  varchar2(128),
locale_imp      number(10),
z_order         number(5),
area            number(12),
perimeter       number(12),
pnt_geom_value  sdo_geometry,
poly_geom_value sdo_geometry
);
create table template_dlm_rail_segment (
id              number(10),
product         number(10),
scale           number(10),
form_id         number(10),
func_id         number(10),
status          number(10),
rail_type       number(10),
z_order         number(5),
length          number(12),
line_geom_value sdo_geometry
);
create table template_dlm_site (
id              number(10),
product         number(10),
scale           number(10),
form_id         number(10),
func_id         number(10),
status          number(10),
z_order         number(5),
primary_name    varchar2(128), -- empty in demo
secondary_name  varchar2(128), -- empty in demo
poly_geom_value sdo_geometry
);
create table template_dlm_structure (
id              number(10),
product         number(10),
scale           number(10),
form_id         number(10),
func_id         number(10),
status          number(10),
primary_name    varchar2(128),
secondary_name  varchar2(128), -- empty in demo
way_guid        varchar2(38),
z_order         number(5),
geom_value      sdo_geometry
);
create table template_dlm_vegetation (
id              number(10),
product         number(10),
scale           number(10),
form_id         number(10),
func_id         number(10),
status          number(10),
primary_name    varchar2(128), -- empty in demo
secondary_name  varchar2(128), -- empty in demo
z_order         number(5),
area            number(12),
perimeter       number(12),
poly_geom_value sdo_geometry
);
create table template_dlm_way (
id              number(10),
product         number(10),
scale           number(10),
form_id         number(10),
func_id         number(10),
status          number(10),
way_type        number(10),
primary_name    varchar2(128),
secondary_name  varchar2(128),
route_num       varchar2(32),
z_order         number(5),
length          number(12),
line_geom_value sdo_geometry,
poly_geom_value sdo_geometry
);
create table template_dlm_water (
id              number(10),
product         number(10),
scale           number(10),
form_id         number(10),
func_id         number(10),
status          number(10),
primary_name    varchar2(128),
secondary_name  varchar2(128),
z_order         number(5),
area            number(12),
perimeter       number(12),
length          number(12),
line_geom_value sdo_geometry,
poly_geom_value sdo_geometry
);
create table template_dlm_water_single (
id              number(10),
product         number(10),
scale           number(10),
form_id         number(10),
func_id         number(10),
status          number(10),
z_order         number(5),
length          number(12),
line_geom_value sdo_geometry
);
spool off