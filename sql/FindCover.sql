undefine X
undefine Y
select guid,func_id,form_id from DLM_0_ARTIFICIAL t where sdo_contains(t.poly_geom_value,SDO_GEOMETRY(2001, 2157, SDO_POINT_TYPE(&&X, &&Y, NULL), NULL, NULL))='TRUE';
select guid,func_id,form_id from DLM_0_BUILDING   t where sdo_contains(t.poly_geom_value,SDO_GEOMETRY(2001, 2157, SDO_POINT_TYPE(&&X, &&Y, NULL), NULL, NULL))='TRUE';
select guid,func_id,form_id from DLM_0_EXPOSED    t where sdo_contains(t.poly_geom_value,SDO_GEOMETRY(2001, 2157, SDO_POINT_TYPE(&&X, &&Y, NULL), NULL, NULL))='TRUE';
select guid,func_id,form_id from DLM_0_SITE       t where sdo_contains(t.poly_geom_value,SDO_GEOMETRY(2001, 2157, SDO_POINT_TYPE(&&X, &&Y, NULL), NULL, NULL))='TRUE';
select guid,func_id,form_id from DLM_0_VEGETATION t where sdo_contains(t.poly_geom_value,SDO_GEOMETRY(2001, 2157, SDO_POINT_TYPE(&&X, &&Y, NULL), NULL, NULL))='TRUE';
select guid,func_id,form_id from DLM_0_WATER      t where sdo_contains(t.poly_geom_value,SDO_GEOMETRY(2001, 2157, SDO_POINT_TYPE(&&X, &&Y, NULL), NULL, NULL))='TRUE';
select guid,func_id,form_id from DLM_0_WAY        t where sdo_contains(t.poly_geom_value,SDO_GEOMETRY(2001, 2157, SDO_POINT_TYPE(&&X, &&Y, NULL), NULL, NULL))='TRUE';
select guid,func_id,form_id from DLM_0_STRUCTURE  t where 
t.geom_value.sdo_gtype=2003 and
sdo_contains(t.geom_value,SDO_GEOMETRY(2001, 2157, SDO_POINT_TYPE(&&X, &&Y, NULL), NULL, NULL))='TRUE';
undefine X
undefine Y