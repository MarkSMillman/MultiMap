spool CopyDLM0FromPrime2.log
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------

create or replace FUNCTION To2D
(
geom SDO_GEOMETRY
) RETURN SDO_GEOMETRY AS

nDims NUMBER;
ords SDO_ORDINATE_ARRAY;
elms SDO_ELEM_INFO_ARRAY;
nOrds NUMBER;
point SDO_POINT_TYPE;
idx NUMBER := 1;
newIdx NUMBER := 1;
gType NUMBER;
offSet NUMBER;
BEGIN

  if (geom is null) then
    return geom;
  end if;

  nDims := geom.get_dims();

  if (nDims < 3 ) then
    return geom;  -- 3D, 4D Only
  end if;
  
  gType := geom.get_gtype();
  if ( gType = 1 ) then
      nOrds := 2;
	  point := sdo_point_type(geom.sdo_point.X,geom.sdo_point.Y,geom.sdo_point.Z);
	  point.X := geom.sdo_point.X;
	  point.Y := geom.sdo_point.Y;
	  gType := gType + 2000;
	  return sdo_geometry(gType, geom.sdo_srid, point, NULL, NULL);
  else  
	  nOrds := geom.sdo_ordinates.count;
	  ords := sdo_ordinate_array();
	  ords.extend(nOrds - (nOrds/nDims));
  
	  while (idx <= nOrds) loop
		ords(newIdx) := geom.sdo_ordinates(idx);
		ords(newIdx + 1) := geom.sdo_ordinates(idx + 1);
		idx := idx + nDims;
		newIdx := newIdx + nDims - 1;
	  end loop;

  
    -- fix up elment info
    nOrds := geom.sdo_elem_info.count;
    elms := sdo_elem_info_array();
    elms.extend(nOrds);
    idx := 1;
    while (idx <= nOrds) loop
      if ((idx mod 3 = 1) AND idx <> 1) then
        offSet := (geom.sdo_elem_info(idx) - 1) / nDims;
        offset := offSet * (nDims - 2);
        elms(idx) := geom.sdo_elem_info(idx) - offSet;
      else
        elms(idx) := geom.sdo_elem_info(idx);
      end if;
      idx := idx + 1;
    end loop;
  end if;
  gType := gType + 2000;
  return sdo_geometry(gType, geom.sdo_srid, geom.sdo_point, elms, ords);
END;
/
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
insert into dlm_0_locale 
      (id,guid,product,scale,form_id,func_id,status,primary_name,secondary_name,locale_imp,z_order,pnt_geom_value,poly_geom_value)
	  select dlm_0_locale_seq.nextval,guid,0,1,form_id,func_id,status_id,english_name_value,irish_name_value,locale_imp_value,0,
	         To2D(pnt_geom_value),To2D(poly_geom_value)
      from prime2_supply_demo.locale gw;
	  
update dlm_0_locale set primary_name=secondary_name,secondary_name=null where primary_name is null and secondary_name is not null;
insert into dlm_0_locale_prime2 (dlm_id,prime2_guid) select id,guid from dlm_0_locale;
insert into user_sdo_geom_metadata select 'DLM_0_LOCALE','PNT_GEOM_VALUE',diminfo,srid from user_sdo_geom_metadata where table_name like 'TEMPLATE_2D';
insert into user_sdo_geom_metadata select 'DLM_0_LOCALE','POLY_GEOM_VALUE',diminfo,srid from user_sdo_geom_metadata where table_name like 'TEMPLATE_2D';
commit;
create unique index dlm_0_locale_pk on dlm_0_locale(id);
create index dlm_0_locale_tsdx on dlm_0_locale(pnt_geom_value) indextype is mdsys.spatial_index;
create index dlm_0_locale_psdx on dlm_0_locale(poly_geom_value) indextype is mdsys.spatial_index;
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
insert into dlm_0_artificial 
      (id,guid,product,scale,form_id,func_id,status,primary_name,secondary_name,z_order,poly_geom_value)
	  select dlm_0_artificial_seq.nextval,guid,0,1,form_id,func_id,status_id,english_name_value,irish_name_value,z_order_value,To2D(poly_geom_value)
      from prime2_supply_demo.artificial gw;
	  
update dlm_0_artificial set primary_name=secondary_name,secondary_name=null where primary_name is null and secondary_name is not null;
insert into dlm_0_artificial_prime2 (dlm_id,prime2_guid) select id,guid from dlm_0_artificial;
insert into user_sdo_geom_metadata select 'DLM_0_ARTIFICIAL','POLY_GEOM_VALUE',diminfo,srid from user_sdo_geom_metadata where table_name like 'TEMPLATE_2D';
commit;
create unique index dlm_0_artificial_pk on dlm_0_artificial(id);
create index dlm_0_artificial_psdx on dlm_0_artificial(poly_geom_value) indextype is mdsys.spatial_index;
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
insert into dlm_0_building 
      (id,guid,product,scale,form_id,func_id,status,primary_name,secondary_name,building_num,way_guid,z_order,pnt_geom_value,poly_geom_value)
	  select dlm_0_building_seq.nextval,guid,0,1,form_id,func_id,status_id,english_name_value,irish_name_value,bld_no_value,way_seg_value,z_order_value,
	         To2D(pnt_geom_value),To2D(poly_geom_value)
      from prime2_supply_demo.building gw;
	  
update dlm_0_building set primary_name=secondary_name,secondary_name=null where primary_name is null and secondary_name is not null;
insert into dlm_0_building_prime2 (dlm_id,prime2_guid) select id,guid from dlm_0_building;
insert into user_sdo_geom_metadata select 'DLM_0_BUILDING','PNT_GEOM_VALUE',diminfo,srid from user_sdo_geom_metadata where table_name like 'TEMPLATE_2D';
insert into user_sdo_geom_metadata select 'DLM_0_BUILDING','POLY_GEOM_VALUE',diminfo,srid from user_sdo_geom_metadata where table_name like 'TEMPLATE_2D';
commit;
create unique index dlm_0_building_pk on dlm_0_building(id);
create index dlm_0_building_tsdx on dlm_0_building(pnt_geom_value) indextype is mdsys.spatial_index;
create index dlm_0_building_psdx on dlm_0_building(poly_geom_value) indextype is mdsys.spatial_index;
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
insert into dlm_0_exposed 
      (id,guid,product,scale,form_id,func_id,status,primary_name,secondary_name,z_order,poly_geom_value)
	  select dlm_0_exposed_seq.nextval,guid,0,1,form_id,func_id,status_id,english_name_value,irish_name_value,z_order_value,To2D(poly_geom_value)
      from prime2_supply_demo.exposed gw;
	  
update dlm_0_exposed set primary_name=secondary_name,secondary_name=null where primary_name is null and secondary_name is not null;
insert into dlm_0_exposed_prime2 (dlm_id,prime2_guid) select id,guid from dlm_0_exposed;
insert into user_sdo_geom_metadata select 'DLM_0_EXPOSED','POLY_GEOM_VALUE',diminfo,srid from user_sdo_geom_metadata where table_name like 'TEMPLATE_2D';
commit;
create unique index dlm_0_exposed_pk on dlm_0_exposed(id);
create index dlm_0_exposed_psdx on dlm_0_exposed(poly_geom_value) indextype is mdsys.spatial_index;
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
insert into dlm_0_division 
      (id,guid,product,scale,form_id,func_id,status,z_order,line_geom_value)
	  select dlm_0_division_seq.nextval,guid,0,1,form_id,func_id,0,z_order_value,To2D(line_geom_value)
      from prime2_supply_demo.division gw;
	  
insert into dlm_0_division_prime2 (dlm_id,prime2_guid) select id,guid from dlm_0_division;
insert into user_sdo_geom_metadata select 'DLM_0_DIVISION','LINE_GEOM_VALUE',diminfo,srid from user_sdo_geom_metadata where table_name like 'TEMPLATE_2D';
commit;
create unique index dlm_0_division_pk on dlm_0_division(id);
create index dlm_0_division_lsdx on dlm_0_division(line_geom_value) indextype is mdsys.spatial_index;

-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
insert into dlm_0_rail_segment 
      (id,guid,product,scale,form_id,func_id,status,rail_type,z_order,line_geom_value)
	  select dlm_0_rail_segment_seq.nextval,guid,0,1,0,0,status_id,rail_type_id,z_order_value,To2D(line_geom_value)
      from prime2_supply_demo.rail_network_segment gw;
	  
insert into dlm_0_rail_segment_prime2 (dlm_id,prime2_guid) select id,guid from dlm_0_rail_segment;
insert into user_sdo_geom_metadata select 'DLM_0_RAIL_SEGMENT','LINE_GEOM_VALUE',diminfo,srid from user_sdo_geom_metadata where table_name like 'TEMPLATE_2D';
commit;
create unique index dlm_0_rail_segment_pk on dlm_0_rail_segment(id);
create index dlm_0_rail_segment_lsdx on dlm_0_rail_segment(line_geom_value) indextype is mdsys.spatial_index;
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
insert into dlm_0_site 
      (id,guid,product,scale,form_id,func_id,status,primary_name,secondary_name,poly_geom_value)
	  select dlm_0_site_seq.nextval,guid,0,1,form_id,func_id,status_id,english_name_value,irish_name_value,To2D(poly_geom_value)
      from prime2_supply_demo.site gw;
	  
update dlm_0_site set primary_name=secondary_name,secondary_name=null where primary_name is null and secondary_name is not null;
insert into dlm_0_site_prime2 (dlm_id,prime2_guid) select id,guid from dlm_0_site;
insert into user_sdo_geom_metadata select 'DLM_0_SITE','POLY_GEOM_VALUE',diminfo,srid from user_sdo_geom_metadata where table_name like 'TEMPLATE_2D';
commit;
create unique index dlm_0_site_pk on dlm_0_site(id);
create index dlm_0_site_psdx on dlm_0_site(poly_geom_value) indextype is mdsys.spatial_index;
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
insert into dlm_0_structure 
      (id,guid,product,scale,form_id,func_id,status,primary_name,secondary_name,way_guid,z_order,geom_value)
	  select dlm_0_structure_seq.nextval,guid,0,1,form_id,func_id,status_id,english_name_value,irish_name_value,way_seg_value,z_order_value,To2D(geom_value)
      from prime2_supply_demo.structure gw;
	  
update dlm_0_structure set primary_name=secondary_name,secondary_name=null where primary_name is null and secondary_name is not null;
insert into dlm_0_structure_prime2 (dlm_id,prime2_guid) select id,guid from dlm_0_structure;
insert into user_sdo_geom_metadata select 'DLM_0_STRUCTURE','GEOM_VALUE',diminfo,srid from user_sdo_geom_metadata where table_name like 'TEMPLATE_2D';
commit;
create unique index dlm_0_structure_pk on dlm_0_structure(id);
create index dlm_0_structure_tsdx on dlm_0_structure(geom_value) indextype is mdsys.spatial_index;
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
insert into dlm_0_vegetation 
      (id,guid,product,scale,form_id,func_id,status,primary_name,secondary_name,z_order,poly_geom_value)
	  select dlm_0_vegetation_seq.nextval,guid,0,1,form_id,func_id,status_id,english_name_value,irish_name_value,z_order_value,To2D(poly_geom_value)
      from prime2_supply_demo.vegetation gw;
	  
update dlm_0_vegetation set primary_name=secondary_name,secondary_name=null where primary_name is null and secondary_name is not null;
insert into dlm_0_vegetation_prime2 (dlm_id,prime2_guid) select id,guid from dlm_0_vegetation;
insert into user_sdo_geom_metadata select 'DLM_0_VEGETATION','POLY_GEOM_VALUE',diminfo,srid from user_sdo_geom_metadata where table_name like 'TEMPLATE_2D';
commit;
create unique index dlm_0_vegetation_pk on dlm_0_vegetation(id);
create index dlm_0_vegetation_psdx on dlm_0_vegetation(poly_geom_value) indextype is mdsys.spatial_index;
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
insert into dlm_0_way 
      (id,product,scale,form_id,func_id,status,way_type,primary_name,secondary_name,route_num,z_order,line_geom_value,poly_geom_value)
	  select generalized_id,0,1,form_id,func_id,status_id,way_type,english_name,irish_name,route_name,z_order_value,To2D(line_geom_value),To2D(poly_geom_value)
      from prime2_enhance.generalized_way gw
	  ;
	  
update dlm_0_way set primary_name=secondary_name,secondary_name=null where primary_name is null and secondary_name is not null;

insert into dlm_0_way_prime2 (dlm_id,prime2_guid) select generalized_id,guid from prime2_enhance.generalized_way_members gm
       where exists ( select null from dlm_0_way dw where dw.id=generalized_id);
insert into user_sdo_geom_metadata select 'DLM_0_WAY','LINE_GEOM_VALUE',diminfo,srid from user_sdo_geom_metadata where table_name like 'TEMPLATE_2D';
insert into user_sdo_geom_metadata select 'DLM_0_WAY','POLY_GEOM_VALUE',diminfo,srid from user_sdo_geom_metadata where table_name like 'TEMPLATE_2D';
commit;
create unique index dlm_0_way_pk on dlm_0_way(id);
create index dlm_0_way_lsdx on dlm_0_way(line_geom_value) indextype is mdsys.spatial_index;
create index dlm_0_way_psdx on dlm_0_way(poly_geom_value) indextype is mdsys.spatial_index;
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
insert into dlm_0_water 
      (id,guid,product,scale,form_id,func_id,status,primary_name,secondary_name,z_order,line_geom_value,poly_geom_value)
	  select dlm_0_water_seq.nextval,guid,0,1,form_id,func_id,status_id,english_name_value,irish_name_value,z_order_value,To2D(line_geom_value),To2D(poly_geom_value)
      from prime2_enhance.water gw;
	  
update dlm_0_water set primary_name=secondary_name,secondary_name=null where primary_name is null and secondary_name is not null;
insert into dlm_0_water_prime2 (dlm_id,prime2_guid) select id,guid from dlm_0_water;
insert into user_sdo_geom_metadata select 'DLM_0_WATER','LINE_GEOM_VALUE',diminfo,srid from user_sdo_geom_metadata where table_name like 'TEMPLATE_2D';
insert into user_sdo_geom_metadata select 'DLM_0_WATER','POLY_GEOM_VALUE',diminfo,srid from user_sdo_geom_metadata where table_name like 'TEMPLATE_2D';
create unique index dlm_0_water_pk on dlm_0_water(id);
create index dlm_0_water_lsdx on dlm_0_water(line_geom_value) indextype is mdsys.spatial_index;
create index dlm_0_water_psdx on dlm_0_water(poly_geom_value) indextype is mdsys.spatial_index;
commit;
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
insert into dlm_0_water_single 
      (id,guid,product,scale,form_id,func_id,status,z_order,line_geom_value)
	  select dlm_0_water_single_seq.nextval,guid,0,1,form_id,0,status_id,z_order_value,To2D(line_geom_value)
      from prime2_supply_demo.water_single_stream gw;
	  
insert into dlm_0_water_single_prime2 (dlm_id,prime2_guid) select id,guid from dlm_0_water_single;
insert into user_sdo_geom_metadata select 'DLM_0_WATER_SINGLE','LINE_GEOM_VALUE',diminfo,srid from user_sdo_geom_metadata where table_name like 'TEMPLATE_2D';
create unique index dlm_0_water_single_pk on dlm_0_water_single(id);
create index dlm_0_water_single_lsdx on dlm_0_water_single(line_geom_value) indextype is mdsys.spatial_index;
commit;
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
update dlm_0_artificial   set area      = sdo_geom.sdo_area(poly_geom_value,1.0);
update dlm_0_artificial   set perimeter = sdo_geom.sdo_length(poly_geom_value,1.0);
update dlm_0_artificial   set length    = sdo_geom.sdo_length(line_geom_value,1.0);
commit;
update dlm_0_exposed      set area      = sdo_geom.sdo_area(poly_geom_value,1.0);
update dlm_0_exposed      set perimeter = sdo_geom.sdo_length(poly_geom_value,1.0);
commit;
update dlm_0_division     set length    = sdo_geom.sdo_length(line_geom_value,1.0);
commit;
update dlm_0_locale       set area      = sdo_geom.sdo_area(poly_geom_value,1.0);
update dlm_0_locale       set perimeter = sdo_geom.sdo_length(poly_geom_value,1.0);
commit;
update dlm_0_rail_segment set length    = sdo_geom.sdo_length(line_geom_value,1.0);
commit;
update dlm_0_vegetation   set area      = sdo_geom.sdo_area(poly_geom_value,1.0);
update dlm_0_vegetation   set perimeter = sdo_geom.sdo_length(poly_geom_value,1.0);
commit;
update dlm_0_way          set length    = sdo_geom.sdo_length(line_geom_value,1.0);
commit;
update dlm_0_water        set area      = sdo_geom.sdo_area(poly_geom_value,1.0);
update dlm_0_water        set perimeter = sdo_geom.sdo_length(poly_geom_value,1.0);
update dlm_0_water        set length    = sdo_geom.sdo_length(line_geom_value,1.0);
commit;
update dlm_0_water_single set length    = sdo_geom.sdo_length(line_geom_value,1.0);
commit;

spool off