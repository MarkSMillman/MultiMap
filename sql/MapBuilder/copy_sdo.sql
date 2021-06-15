delete from user_sdo_geom_metadata;
delete from user_sdo_maps;
delete from user_sdo_themes;
delete from user_sdo_styles;
delete from user_sdo_cached_maps;

insert into user_sdo_geom_metadata (select * from bup_sdo_geom_metadata);
insert into user_sdo_maps (select * from bup_sdo_maps);
insert into user_sdo_themes (select * from bup_sdo_themes);
insert into user_sdo_styles (select * from bup_sdo_styles);
insert into user_sdo_cached_maps (select * from bup_sdo_cached_maps);

-- If it worked the way you expected COMMIT not
