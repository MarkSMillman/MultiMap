spool dup_names.log
select '##E='||w.english_name_value,'I='||w.irish_name_value,'R='||w.route_name from way w,  generalized_way g, generalized_way_members m 
        where g.minimum_scale=13000 and m.generalized_id=g.generalized_id and m.guid=w.guid
		 group by '##E='||w.english_name_value,'I='||w.irish_name_value,'R='||w.route_name
		having count(*) > 1
		 order by '##E='||w.english_name_value,'I='||w.irish_name_value,'R='||w.route_name
;
spool off

select count(*) from way w0 where exists 
(select null from way w,  generalized_way g, generalized_way_members m 
 where g.minimum_scale=13000 and m.generalized_id=g.generalized_id and m.guid=w.guid and w0.rowid < w.rowid) 
       and w0.english_name_value is not null or w0.irish_name_value is not null or w0.route_name is not null
;
	   
select guid from way w0 where exists 
(select null from way w,  generalized_way g, generalized_way_members m 
 where g.minimum_scale=13000 and m.generalized_id=g.generalized_id and m.guid=w.guid and
       w0.rowid < w.rowid);