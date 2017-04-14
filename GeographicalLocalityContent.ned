package modules.content;

simple GeographicalLocalityContentDistribution extends content_distribution{

    parameters:
		@class(GeographicalLocalityContentDistribution);
		
		string request_patterns_file;
		string client_to_region_map_file;
}
