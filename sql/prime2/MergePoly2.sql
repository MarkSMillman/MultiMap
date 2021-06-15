create table md as  (select dlm_id from dlm_0_way_prime2 group by dlm_id having count(*) > 1) ;
update dlm_0_way dw set poly_geom_value=
       (select to2d(sdo_aggr_union(mdsys.sdoaggrtype(p.poly_geom_value,0.05))) 
	          from prime2_enhance.way p, dlm_0_way_prime2 d where d.prime2_guid=p.guid and d.dlm_id=dw.id)
	   where exists (select null from md m where m.dlm_id=dw.id);