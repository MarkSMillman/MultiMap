update generalized_way g set 
(english_name,irish_name,route_name) = 
(select distinct english_name_value,irish_name_value,route_name from way w, generalized_way_members m 
        where g.minimum_scale=13000 and m.generalized_id=g.generalized_id and m.guid=w.guid
		and w.english_name_value is not null or w.irish_name_value is not null or w.route_name is not null) 
where g.minimum_scale=13000
;
