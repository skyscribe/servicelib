///refactor to 
namespace{
	const std::unordered_map<int, std::string> rp3Map = {
		{EBbBusLinkId_RP30, "RP3_0"},
	    {EBbBusLinkId_RP31, "RP3_1"},
	    {EBbBusLinkId_RP32, "RP3_2"},
	    {EBbBusLinkId_RP33, "RP3_3"},
	    {EBbBusLinkId_RP34, "RP3_4"},
	    {EBbBusLinkId_RP35, "RP3_5"},
	    {EBbBusLinkId_RP36, "RP3_6"},
	    {EBbBusLinkId_RP37, "RP3_7"},
	    {EBbBusLinkId_RP38, "RP3_8"},
	    {EBbBusLinkId_RP39, "RP3_9"},
	    {EBbBusLinkId_RP310, "RP3_10"},
	    {EBbBusLinkId_RP311, "RP3_11"},
	};
    std::unorderd_map<int, std::string> idPortMap2To1 = {
	    {EBbBusLinkId_Cpri0, "CPRI_0"},
	    {EBbBusLinkId_Cpri1, "CPRI_0"},
	    {EBbBusLinkId_Cpri2, "CPRI_2"},
	    {EBbBusLinkId_Cpri3, "CPRI_2"},
	    {EBbBusLinkId_Cpri4, "CPRI_4"},
	    {EBbBusLinkId_Cpri5, "CPRI_4"},
    };
	
    std::unorderd_map<int, std::string> idPortMap3To1 = {
	    {EBbBusLinkId_Cpri0, "CPRI_0"},
	    {EBbBusLinkId_Cpri1, "CPRI_0"},
	    {EBbBusLinkId_Cpri2, "CPRI_0"},
	    {EBbBusLinkId_Cpri3, "CPRI_4"},
	    {EBbBusLinkId_Cpri4, "CPRI_4"},
	    {EBbBusLinkId_Cpri5, "CPRI_4"},
    };
	
	struct raii{
		struct raii(){
			idPortMap2To1.insert(rp3Map);
			idPortMap3To1.insert(rp3Map);
		}
	}inst;
}
std::string BBSwitchRoutingConfigTask::convertBBLinkIdToPortStringFor9Dot8Gbps(EBbBusLinkId bbBusLinkId, ECpriLinkRate linkRate)
{
    const auto& mapToSearch = idPortMap2To1;
    if (linkRate == ECpriLinkRate_9830_4Mbps_3_to_1){
		mapToSearch = idPortMap3To1;
    }else if (linkRate != ECpriLinkRate_6144_0Mbps_2_to_1 && linkRate != ECpriLinkRate_9830_4Mbps_2_to_1){
		//TODO: Exceptional case handling here!
		return "";
	}else{
		const auto& it = mapToSearch.find(linkRate);
		if (it != mapToSearch.end()){
			return it->second;
		}else{
			//TODO: exceptional case handling here!
			logger->log_warning("cannot convert BBus link(%d) to port string", bbBusLinkId);
			return "";
		}
	}
}

////was
std::string BBSwitchRoutingConfigTask::convertBBLinkIdToPortStringFor9Dot8Gbps(EBbBusLinkId bbBusLinkId, ECpriLinkRate linkRate)
{
    std::string port = "";

    if (linkRate == ECpriLinkRate_6144_0Mbps_2_to_1 || linkRate == ECpriLinkRate_9830_4Mbps_2_to_1)
    {
        switch (bbBusLinkId)
        {
            case EBbBusLinkId_Cpri0: port = "CPRI_0"; break;
            case EBbBusLinkId_Cpri1: port = "CPRI_0"; break;
            case EBbBusLinkId_Cpri2: port = "CPRI_2"; break;
            case EBbBusLinkId_Cpri3: port = "CPRI_2"; break;
            case EBbBusLinkId_Cpri4: port = "CPRI_4"; break;
            case EBbBusLinkId_Cpri5: port = "CPRI_4"; break;
            case EBbBusLinkId_RP30:  port = "RP3_0";  break;
            case EBbBusLinkId_RP31:  port = "RP3_1";  break;
            case EBbBusLinkId_RP32:  port = "RP3_2";  break;
            case EBbBusLinkId_RP33:  port = "RP3_3";  break;
            case EBbBusLinkId_RP34:  port = "RP3_4";  break;
            case EBbBusLinkId_RP35:  port = "RP3_5";  break;
            case EBbBusLinkId_RP36:  port = "RP3_6";  break;
            case EBbBusLinkId_RP37:  port = "RP3_7";  break;
            case EBbBusLinkId_RP38:  port = "RP3_8";  break;
            case EBbBusLinkId_RP39:  port = "RP3_9";  break;
            case EBbBusLinkId_RP310:  port = "RP3_10"; break;
            case EBbBusLinkId_RP311:  port = "RP3_11"; break;

            default:
                logger->log_warning("cannot convert BBus link(%d) to port string", bbBusLinkId);
                break;
        }
    }
    else if (linkRate == ECpriLinkRate_9830_4Mbps_3_to_1)
    {
        switch (bbBusLinkId)
        {
            case EBbBusLinkId_Cpri0: port = "CPRI_0"; break;
            case EBbBusLinkId_Cpri1: port = "CPRI_0"; break;
            case EBbBusLinkId_Cpri2: port = "CPRI_0"; break;
            case EBbBusLinkId_Cpri3: port = "CPRI_4"; break;
            case EBbBusLinkId_Cpri4: port = "CPRI_4"; break;
            case EBbBusLinkId_Cpri5: port = "CPRI_4"; break;
            case EBbBusLinkId_RP30:  port = "RP3_0";  break;
            case EBbBusLinkId_RP31:  port = "RP3_1";  break;
            case EBbBusLinkId_RP32:  port = "RP3_2";  break;
            case EBbBusLinkId_RP33:  port = "RP3_3";  break;
            case EBbBusLinkId_RP34:  port = "RP3_4";  break;
            case EBbBusLinkId_RP35:  port = "RP3_5";  break;
            case EBbBusLinkId_RP36:  port = "RP3_6";  break;
            case EBbBusLinkId_RP37:  port = "RP3_7";  break;
            case EBbBusLinkId_RP38:  port = "RP3_8";  break;
            case EBbBusLinkId_RP39:  port = "RP3_9";  break;
            case EBbBusLinkId_RP310:  port = "RP3_10"; break;
            case EBbBusLinkId_RP311:  port = "RP3_11"; break;

            default:
                logger->log_warning("cannot convert BBus link(%d) to port string", bbBusLinkId);
                break;
        }
    }

    return port;
}

