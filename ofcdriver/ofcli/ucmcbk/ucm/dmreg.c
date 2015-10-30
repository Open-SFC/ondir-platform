/**
 * Copyright (c) 2012, 2013  Freescale.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

/*
 * $Source: /cvsroot/fslrepo/sdn-of/controller/ofcli/ucmcbk/ucm/dmreg.c,v $
 * $Revision: 1.1.1.2.2.49.2.1 $
 * $Date: 2014/06/26 06:10:10 $
 */ 

/**************************************************************************/
/* Copyright (c) 2010, 2011 Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Freescale Semiconductor nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *
 * ALTERNATIVELY, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") as published by the Free Software
 * Foundation, either version 2 of that License or (at your option) any
 * later version.
 *
 * THIS SOFTWARE IS PROVIDED BY Freescale Semiconductor ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Freescale Semiconductor BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
/****************************************************************************
 *  File                : 
 *  Description         : This file contains source code implmenting 
 *                        configuration middleware data model tree.
 *  Revision History    :
 *    version    Date          Author           Change Description
****************************************************************************/
#ifdef OF_CM_SUPPORT

#include "openflow.h" 
#include "cbcmninc.h" 
#include "igwincl.h"
#include "dpcbk.h"

extern int32_t CM_RegisterDMNodes (void);



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Function Name:
 * Description:
 * Input:
 * Output:
 * Result:
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32_t CM_RegisterDMNodes ()
{
  int32_t return_value;

   CM_CBK_DEBUG_PRINT (" Entering ");
	 
	//on_director
	 return_value = cm_dm_register_data_elements (CM_ON_DIRECTOR_APPL_ID,
				 CM_SELF_CHANNEL_ID,CM_DM_ROOT_NODE_PATH,
				 (sizeof (on_directorDataElements)/
					sizeof (struct cm_dm_data_element)),
				 &(on_directorDataElements[0]));
	 if (return_value != OF_SUCCESS)
	 {
			CM_CBK_DEBUG_PRINT (" RegisterDataElements : on_directorDataElements, Error : %d", return_value);
			return OF_FAILURE;
	 } 

	 //View
	 return_value =
		 cm_dm_register_data_elements (CM_ON_DIRECTOR_VIEW_APPL_ID, CM_DP_CHANNEL_ID,
				 CM_DM_ON_DIRECTOR_VIEW_PATH,
				 (sizeof (viewDataElements) /
				  sizeof (struct cm_dm_data_element)),
				 &(viewDataElements[0]));
	 if (return_value != OF_SUCCESS)
	 {
      CM_CBK_DEBUG_PRINT ("viewDataElements, Error : %d", return_value);
		 return OF_FAILURE;
	 }


         //Namespace
         return_value =
                 cm_dm_register_data_elements (CM_ON_DIRECTOR_NAMESPACE_APPL_ID, CM_DP_CHANNEL_ID,
                                 CM_DM_ON_DIRECTOR_NAMESPACE_PATH,
                                 (sizeof (namespaceDataElements) /
                                  sizeof (struct cm_dm_data_element)),
                                 &(namespaceDataElements[0]));
         if (return_value != OF_SUCCESS)
         {
                 CM_CBK_DEBUG_PRINT ("NamespaceDataElements, Error : %d", return_value);
                 return OF_FAILURE;
         }
        //apps
	return_value =
                 cm_dm_register_data_elements(CM_ON_DIRECTOR_NAMESPACE_APPS_APPL_ID,CM_DP_CHANNEL_ID,
                                         CM_DM_ON_DIRECTOR_NAMESPACE_APPS_PATH,
                                         (sizeof (appsDataElements) /
                                                sizeof (struct cm_dm_data_element)),
                                         &(appsDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
           CM_CBK_DEBUG_PRINT ("of apps Elements, Error : %d", return_value );
           return OF_FAILURE;
        }
 

        //arp
	return_value =
                 cm_dm_register_data_elements(CM_ON_DIRECTOR_NAMESPACE_APPS_ARP_APPL_ID,CM_DP_CHANNEL_ID,
                                         CM_DM_ON_DIRECTOR_NAMESPACE_APPS_ARP_PATH,
                                         (sizeof (arpDataElements) /
                                                sizeof (struct cm_dm_data_element)),
                                         &(arpDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
           CM_CBK_DEBUG_PRINT ("arp entries, Error : %d", return_value );
           return OF_FAILURE;
        }
      
       //mcast
       return_value =
                 cm_dm_register_data_elements(CM_ON_DIRECTOR_NAMESPACE_APPS_MCAST_APPL_ID,CM_DP_CHANNEL_ID,CM_DM_ON_DIRECTOR_NAMESPACE_APPS_MCAST_PATH,
                                         (sizeof (mcastDataElements) /
                                                sizeof (struct cm_dm_data_element)),
                                         &(mcastDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
           CM_CBK_DEBUG_PRINT ("multicast entries, Error : %d", return_value );
           return OF_FAILURE;
        }

/* LIM Stats */
  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_NAMESPACE_LIM_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_NAMESPACE_LIM_PATH,
        (sizeof (limDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(limDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("LIM stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_NAMESPACE_LIM_ARP_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_NAMESPACE_LIM_ARP_PATH,
        (sizeof (limarpDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(limarpDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("LIM stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_NAMESPACE_LIM_ARP_STATS_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_NAMESPACE_LIM_ARP_STATS_PATH,
        (sizeof (arplimstatsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(arplimstatsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("LIM stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_NAMESPACE_LIM_ARP_STATS_ARPSTATS_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_NAMESPACE_LIM_ARP_STATS_ARPSTATS_PATH,
        (sizeof (arpstatsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(arpstatsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("LIM stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }
  /* LIM Route */
  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_NAMESPACE_LIM_ROUTE_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_NAMESPACE_LIM_ROUTE_PATH,
        (sizeof (routeDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(routeDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("LIM stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_NAMESPACE_LIM_ROUTE_STATS_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_NAMESPACE_LIM_ROUTE_STATS_PATH,
        (sizeof (routelimstatsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(routelimstatsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("LIM stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_NAMESPACE_LIM_ROUTE_STATS_ROUTESTATS_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_NAMESPACE_LIM_ROUTE_STATS_ROUTESTATS_PATH,
        (sizeof (routestatsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(routestatsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("LIM stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }
  /* LIM Firewall stats */
  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_NAMESPACE_LIM_FIREWALL_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_NAMESPACE_LIM_FIREWALL_PATH,
        (sizeof (limfirewallstatsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(limfirewallstatsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("LIM stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_NAMESPACE_LIM_FIREWALL_STATS_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_NAMESPACE_LIM_FIREWALL_STATS_PATH,
        (sizeof (fwstatsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(fwstatsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("LIM stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_NAMESPACE_LIM_FIREWALL_STATS_FWCONNSTATS_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_NAMESPACE_LIM_FIREWALL_STATS_FWCONNSTATS_PATH,
        (sizeof (fwconnstatsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(fwconnstatsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("LIM stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_NAMESPACE_LIM_FIREWALL_STATS_FWFWD_RULESTATS_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_NAMESPACE_LIM_FIREWALL_STATS_FWFWD_RULESTATS_PATH,
        (sizeof (fw_fwdstatsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(fw_fwdstatsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("LIM stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

/* LIM Stats */

#ifdef CNTRL_NETWORK_ELEMENT_MAPPER_SUPPORT
   //NEM
   return_value =
      cm_dm_register_data_elements (CM_ON_DIRECTOR_NEM_APPL_ID, CM_DP_CHANNEL_ID,
            CM_DM_ON_DIRECTOR_NEM_PATH,
            (sizeof (nemDataElements) /
             sizeof (struct cm_dm_data_element)),
            &(nemDataElements[0]));
   if (return_value != OF_SUCCESS)
   {
      CM_CBK_DEBUG_PRINT ("NEM DataElements, Error : %d", return_value);
      return OF_FAILURE;
   }


   //NS2NSID
   return_value =
      cm_dm_register_data_elements (CM_ON_DIRECTOR_NEM_NS2NSID_APPL_ID, CM_DP_CHANNEL_ID,
            CM_DM_ON_DIRECTOR_NEM_NS2NSID_PATH,
            (sizeof (ns2nsidDataElements) /
             sizeof (struct cm_dm_data_element)),
            &(ns2nsidDataElements[0]));
   if (return_value != OF_SUCCESS)
   {
      CM_CBK_DEBUG_PRINT ("Ns2nsid DataElements, Error : %d", return_value);
      return OF_FAILURE;
   }


   //NSID2NS
   return_value =
      cm_dm_register_data_elements (CM_ON_DIRECTOR_NEM_NSID2NS_APPL_ID, CM_DP_CHANNEL_ID,
            CM_DM_ON_DIRECTOR_NEM_NSID2NS_PATH,
            (sizeof (nsid2nsDataElements) /
             sizeof (struct cm_dm_data_element)),
            &(nsid2nsDataElements[0]));
   if (return_value != OF_SUCCESS)
   {
      CM_CBK_DEBUG_PRINT ("Nsid2ns DataElements, Error : %d", return_value);
      return OF_FAILURE;
   }


   //DPID2NSID
   return_value =
      cm_dm_register_data_elements (CM_ON_DIRECTOR_NEM_DPID2NSID_APPL_ID, CM_DP_CHANNEL_ID,
            CM_DM_ON_DIRECTOR_NEM_DPID2NSID_PATH,
            (sizeof (dpid2nsidDataElements) /
             sizeof (struct cm_dm_data_element)),
            &(dpid2nsidDataElements[0]));
   if (return_value != OF_SUCCESS)
   {
      CM_CBK_DEBUG_PRINT ("dpid2nsid DataElements, Error : %d", return_value);
      return OF_FAILURE;
   }


   //NSID2DPID
   return_value =
      cm_dm_register_data_elements (CM_ON_DIRECTOR_NEM_NSID2DPID_APPL_ID, CM_DP_CHANNEL_ID,
            CM_DM_ON_DIRECTOR_NEM_NSID2DPID_PATH,
            (sizeof (nsid2dpidDataElements) /
             sizeof (struct cm_dm_data_element)),
            &(nsid2dpidDataElements[0]));
   if (return_value != OF_SUCCESS)
   {
      CM_CBK_DEBUG_PRINT ("Nsid2dpid DataElements, Error : %d", return_value);
      return OF_FAILURE;
   }


   //DPID2PETH
   return_value =
      cm_dm_register_data_elements (CM_ON_DIRECTOR_NEM_DPID2PETH_APPL_ID, CM_DP_CHANNEL_ID,
            CM_DM_ON_DIRECTOR_NEM_DPID2PETH_PATH,
            (sizeof (dpid2pethDataElements) /
             sizeof (struct cm_dm_data_element)),
            &(dpid2pethDataElements[0]));
   if (return_value != OF_SUCCESS)
   {
      CM_CBK_DEBUG_PRINT ("DPID2peth DataElements, Error : %d", return_value);
      return OF_FAILURE;
   }


   //PETH2DPID
   return_value =
      cm_dm_register_data_elements (CM_ON_DIRECTOR_NEM_PETH2DPID_APPL_ID, CM_DP_CHANNEL_ID,
            CM_DM_ON_DIRECTOR_NEM_PETH2DPID_PATH,
            (sizeof (peth2dpidDataElements) /
             sizeof (struct cm_dm_data_element)),
            &(peth2dpidDataElements[0]));
   if (return_value != OF_SUCCESS)
   {
      CM_CBK_DEBUG_PRINT ("peth2dpid DataElements, Error : %d", return_value);
      return OF_FAILURE;
   }

#endif /* CNTRL_NETWORK_ELEMENT_MAPPER_SUPPORT */


	//Switch
  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_ONSWITCH_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_ONSWITCH_PATH,
        (sizeof (onswitchDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(onswitchDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT (" SwitchDataElements, Error : %d", return_value);
    return OF_FAILURE;
  }

	//Datapath
  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_PATH,
        (sizeof (datapathDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(datapathDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("DatapathDataElements, Error : %d", return_value);
    return OF_FAILURE;
  }

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_TRACE_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_TRACE_PATH,
        (sizeof (traceDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(traceDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT (" trace DataElements, Error : %d", return_value);
    return OF_FAILURE;
  }

	//Port
  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_PORT_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_PORT_PATH,
        (sizeof (portDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(portDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("portDataElements, Error : %d", return_value);
    return OF_FAILURE;   
  }
#if 0
	return_value =
cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_PORT_PORTATTRIBUTE_APPL_ID, CM_DP_CHANNEL_ID,
		CM_DM_ON_DIRECTOR_DATAPATH_PORT_PORTATTRIBUTE_PATH ,
		(sizeof (portattributeDataElements) /
		 sizeof (struct cm_dm_data_element)),
		&(portattributeDataElements[0]));
if (return_value != OF_SUCCESS)
{
	CM_CBK_DEBUG_PRINT ("portattributeDataElements, Error : %d", return_value);
	return OF_FAILURE; 
}
#endif


  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_DPATTRIBUTE_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_DPATTRIBUTE_PATH,
        (sizeof (dpattributeDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(dpattributeDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("dpattributeDataElements, Error : %d", return_value);
    return OF_FAILURE; 
  }


#if 0
  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_DPATTRIBUTE_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_DPATTRIBUTE_PATH,
        (sizeof (dpattributeDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(dpattributeDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("dpattributeDataElements, Error : %d", return_value);
    return OF_FAILURE; 
  }


#endif

/*Firewall */

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_FIREWALL_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_FIREWALL_PATH,
        (sizeof (firewallDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(firewallDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("firewall data elements registration, Error : %d", return_value);
    return OF_FAILURE; 
  }


  /*return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_FIREWALL_ACL_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_FIREWALL_ACL_PATH,
        (sizeof (aclDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(aclDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("ACL data elements registration, Error : %d", return_value);
    return OF_FAILURE; 
  }*/

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_FIREWALL_FILTERINPUT_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_FIREWALL_FILTERINPUT_PATH,
        (sizeof (filterinputDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(filterinputDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("ACL data elements registration, Error : %d", return_value);
    return OF_FAILURE; 
  }

//TODO: Need to change FILTEROUTPUT to FILTERFORWARD
  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_FIREWALL_FILTERFORWARD_APPL_ID, 
                             CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_FIREWALL_FILTERFORWARD_PATH,
        (sizeof (filterforwardDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(filterforwardDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("ACL data elements registration, Error : %d", return_value);
    return OF_FAILURE; 
  }

  /** connection templates registration **/
  return_value =
    cm_dm_register_data_elements (
        CM_ON_DIRECTOR_NAMESPACE_APPS_CONNTEMPLATES_APPL_ID,
        CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_NAMESPACE_APPS_CONNTEMPLATES_PATH,
        (sizeof (conntemplatesDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(conntemplatesDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("connection templates registration, Error : %d", return_value);
    return OF_FAILURE;
  }


/*Firewall */


  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_ONSWITCH_SWITCHATTRIBUTE_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_ONSWITCH_SWITCHATTRIBUTE_PATH,
        (sizeof (switchattributeDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(switchattributeDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("switchattributeDataElements, Error : %d", return_value);
    return OF_FAILURE; 
  }


  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_CHANNELS_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_CHANNELS_PATH,
        (sizeof (channelsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(channelsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("channelsDataElements, Error : %d", return_value);
    return OF_FAILURE;
  }
  
  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_AUXCHANNELS_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_AUXCHANNELS_PATH,
        (sizeof (auxchannelsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(auxchannelsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Auxilary ChannelsDataElements, Error : %d", return_value);
    return OF_FAILURE;
  }


//Meters
	return_value =
		cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_METERS_APPL_ID, CM_DP_CHANNEL_ID,
		               CM_DM_ON_DIRECTOR_DATAPATH_METERS_PATH,
				(sizeof (metersDataElements) /
				 sizeof (struct cm_dm_data_element)),
				&(metersDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT (" metersDataElements, Error : %d", return_value);
		return OF_FAILURE;
}

//Meter Record
	return_value =
		cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_METERS_METER_APPL_ID, CM_DP_CHANNEL_ID,
		                CM_DM_ON_DIRECTOR_DATAPATH_METERS_METER_PATH,
				(sizeof (meterrecDataElements) /
				 sizeof (struct cm_dm_data_element)),
				&(meterrecDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT (" meterrecDataElements, Error : %d", return_value);
		return OF_FAILURE;
	}

//Meter Bands
	return_value =
		cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_METERS_METER_BANDS_APPL_ID,CM_DP_CHANNEL_ID,
		                CM_DM_ON_DIRECTOR_DATAPATH_METERS_METER_BANDS_PATH,
				(sizeof (bandsDataElements) /
				 sizeof (struct cm_dm_data_element)),
				&(bandsDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT (" bandsDataElements, Error : %d", return_value);
		return OF_FAILURE;
	}


//Meter Stats
	return_value =
		cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_METERS_STATS_APPL_ID, CM_DP_CHANNEL_ID,
				CM_DM_ON_DIRECTOR_DATAPATH_METERS_STATS_PATH,
				(sizeof (meterstatsDataElements) /
				 sizeof (struct cm_dm_data_element)),
				&(meterstatsDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT (" meterstatsDataElements, Error : %d", return_value);
		return OF_FAILURE;
	}

//Meter Features
	return_value =
		cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_METERS_FEATURES_APPL_ID, CM_DP_CHANNEL_ID,
				CM_DM_ON_DIRECTOR_DATAPATH_METERS_FEATURES_PATH,
				(sizeof (meterfeaturesDataElements) /
				 sizeof (struct cm_dm_data_element)),
				&(meterfeaturesDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT (" meterfeaturesDataElements, Error : %d", return_value);
		return OF_FAILURE;
	}

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_SWITCHINFO_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_SWITCHINFO_PATH,
        (sizeof (swinfoDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(swinfoDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("swinfoDataElements, Error : %d", return_value);
    return OF_FAILURE;
  }

//Stats
  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_STATS_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_STATS_PATH,
        (sizeof (statsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(statsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("statsDataElements, Error : %d", return_value);
    return OF_FAILURE;
  }

//MemPoolStats
	return_value =
cm_dm_register_data_elements (CM_ON_DIRECTOR_MEMPOOLSTATS_APPL_ID, CM_DP_CHANNEL_ID,
		CM_DM_ON_DIRECTOR_MEMPOOLSTATS_PATH,
		(sizeof (mempoolstatsDataElements) /
		 sizeof (struct cm_dm_data_element)),
		&(mempoolstatsDataElements[0]));
if (return_value != OF_SUCCESS)
{
	CM_CBK_DEBUG_PRINT ("mempoolstatsDataElements, Error : %d", return_value);
	return OF_FAILURE;
}





	//PortStats
  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_STATS_PORTSTATS_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_STATS_PORTSTATS_PATH,
        (sizeof (portstatsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(portstatsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("portstatsDataElements, Error : %d", return_value);
    return OF_FAILURE;
  }


	//TableStats
  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_STATS_TABLESTATS_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_STATS_TABLESTATS_PATH,
        (sizeof (tablestatsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(tablestatsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("tablestatsDataElements, Error : %d", return_value);
    return OF_FAILURE;
  }


	//FlowStats
	return_value =
		 cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_STATS_FLOWSTATS_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_DATAPATH_STATS_FLOWSTATS_PATH,
					 (sizeof (flowstatsDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(flowstatsDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
		 CM_CBK_DEBUG_PRINT (" Flowstats_dataElements, Error : %d", return_value);
		 return OF_FAILURE;
	}

      //bind stats
        return_value =
		 cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_STATS_BINDSTATS_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_DATAPATH_STATS_BINDSTATS_PATH,
					 (sizeof (bindstatsDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(bindstatsDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
		 CM_CBK_DEBUG_PRINT (" Bindstats_dataElements, Error : %d", return_value);
		 return OF_FAILURE;
	}
        return_value =
		 cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_STATS_SPDSTATS_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_DATAPATH_STATS_SPDSTATS_PATH,
					 (sizeof (spdstatsDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(spdstatsDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
		 CM_CBK_DEBUG_PRINT (" spdstats_dataElements, Error : %d", return_value);
		 return OF_FAILURE;
	}

return_value =
		 cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_STATS_SASTATS_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_DATAPATH_STATS_SASTATS_PATH,
					 (sizeof (sastatsDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(sastatsDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
		 CM_CBK_DEBUG_PRINT (" Bindstats_dataElements, Error : %d", return_value);
		 return OF_FAILURE;
	}


	//AggregateStats
	return_value =
		 cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_STATS_AGGREGATESTATS_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_DATAPATH_STATS_AGGREGATESTATS_PATH,
					 (sizeof (aggregatestatsDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(aggregatestatsDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
		 CM_CBK_DEBUG_PRINT (" aggregatestatsDataElements, Error : %d", return_value);
		 return OF_FAILURE;
	}


	 //Domain module registration
	return_value =
		 cm_dm_register_data_elements (CM_ON_DIRECTOR_DOMAIN_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_DOMAIN_PATH,
					 (sizeof (domainDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(domainDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
		 CM_CBK_DEBUG_PRINT (" domainDataElement, Error : %d", return_value);
		 return OF_FAILURE;
	}

#if 0
 //Domain table registration
	return_value =
		 cm_dm_register_data_elements (CM_ON_DIRECTOR_DOMAIN_DOMAINTABLE_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_DOMAIN_DOMAINTABLE_PATH,
					 (sizeof (domaintableDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(domaintableDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
		 CM_CBK_DEBUG_PRINT (" domainDataElement, Error : %d", return_value);
		 return OF_FAILURE;
	}

#endif
/*	return_value =
cm_dm_register_data_elements (CM_ON_DIRECTOR_DOMAIN_DOMAINATTRIBUTE_APPL_ID, CM_DP_CHANNEL_ID,
		CM_DM_ON_DIRECTOR_DOMAIN_DOMAINATTRIBUTE_PATH ,
		(sizeof (domainattributeDataElements) /
		 sizeof (struct cm_dm_data_element)),
		&(domainattributeDataElements[0]));
if (return_value != OF_SUCCESS)
{
	CM_CBK_DEBUG_PRINT ("domainattributeDataElements, Error : %d", return_value);
	return OF_FAILURE; 
}*/

#if 1 
	return_value =
cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_PORT_PORTATTRIBUTE_APPL_ID, CM_DP_CHANNEL_ID,
		CM_DM_ON_DIRECTOR_DATAPATH_PORT_PORTATTRIBUTE_PATH ,
		(sizeof (portattributeDataElements) /
		 sizeof (struct cm_dm_data_element)),
		&(portattributeDataElements[0]));
if (return_value != OF_SUCCESS)
{
	CM_CBK_DEBUG_PRINT ("portattributeDataElements, Error : %d", return_value);
	return OF_FAILURE; 
}
#endif



	//Events
       return_value = cm_dm_register_data_elements(CM_ON_DIRECTOR_DATAPATH_EVENTS_APPL_ID, CM_DP_CHANNEL_ID,
                                         CM_DM_ON_DIRECTOR_DATAPATH_EVENTS_PATH,
                                         (sizeof (eventsDataElements) /
                                                sizeof (struct cm_dm_data_element)),
                                         &(eventsDataElements[0]));
        if (return_value!= OF_SUCCESS)
        {
                 CM_CBK_DEBUG_PRINT (" eventsDataElements, Error : %d", return_value);
                 return OF_FAILURE;
        }

       return_value = cm_dm_register_data_elements(CM_ON_DIRECTOR_DATAPATH_IPOPTIONS_APPL_ID, CM_DP_CHANNEL_ID,
                                         CM_DM_ON_DIRECTOR_DATAPATH_IPOPTIONS_PATH,
                                         (sizeof (ipoptionsDataElements) /
                                                sizeof (struct cm_dm_data_element)),
                                         &(ipoptionsDataElements[0]));
        if (return_value!= OF_SUCCESS)
        {
                 CM_CBK_DEBUG_PRINT (" ipOptionsDataElements, Error : %d", return_value);
                 return OF_FAILURE;
        }

/* controller role */

       return_value = cm_dm_register_data_elements(CM_ON_DIRECTOR_DATAPATH_ONDIRECTORROLE_APPL_ID, 
						   CM_DP_CHANNEL_ID,
						   CM_DM_ON_DIRECTOR_DATAPATH_ONDIRECTORROLE_PATH,
	                                           (sizeof (ondirectorroleDataElements)/sizeof (struct cm_dm_data_element)),
	                                           &(ondirectorroleDataElements[0]));
        if (return_value!= OF_SUCCESS)
        {
                 CM_CBK_DEBUG_PRINT (" ondirectorroleDataElements, Error : %d", return_value);
                 return OF_FAILURE;
        }


/* Aysnc configuration message settings */
       return_value = cm_dm_register_data_elements(CM_ON_DIRECTOR_DATAPATH_ASYNCMSGCFG_APPL_ID, 
						   CM_DP_CHANNEL_ID,
						   CM_DM_ON_DIRECTOR_DATAPATH_ASYNCMSGCFG_PATH,
	                                           (sizeof (asyncmsgcfgDataElements)/sizeof (struct cm_dm_data_element)),
	                                           &(asyncmsgcfgDataElements[0]));
        if (return_value!= OF_SUCCESS)
        {
                 CM_CBK_DEBUG_PRINT (" asyncmsgcfgDataElements, Error : %d", return_value);
                 return OF_FAILURE;
        }





//Groups
	return_value =
		cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_GROUPS_APPL_ID, CM_DP_CHANNEL_ID,
		               CM_DM_ON_DIRECTOR_DATAPATH_GROUPS_PATH,
				(sizeof (groupsDataElements) /
				 sizeof (struct cm_dm_data_element)),
				&(groupsDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT (" groupsDataElement, Error : %d", return_value);
		return OF_FAILURE;
}

//Group
	return_value =
		cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_GROUPS_GROUP_APPL_ID, CM_DP_CHANNEL_ID,
		                CM_DM_ON_DIRECTOR_DATAPATH_GROUPS_GROUP_PATH,
				(sizeof (groupdescDataElements) /
				 sizeof (struct cm_dm_data_element)),
				&(groupdescDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT (" group desc DataElement, Error : %d", return_value);
		return OF_FAILURE;
	}

//Bucket
	return_value =
		cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_GROUPS_GROUP_BUCKET_APPL_ID,CM_DP_CHANNEL_ID,
		                CM_DM_ON_DIRECTOR_DATAPATH_GROUPS_GROUP_BUCKET_PATH,
				(sizeof (bucketDataElements) /
				 sizeof (struct cm_dm_data_element)),
				&(bucketDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT (" group desc DataElement, Error : %d", return_value);
		return OF_FAILURE;
	}

//Actions

	return_value =
		cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_GROUPS_GROUP_BUCKET_ACTION_APPL_ID,CM_DP_CHANNEL_ID,
		                CM_DM_ON_DIRECTOR_DATAPATH_GROUPS_GROUP_BUCKET_ACTION_PATH,
				(sizeof (actionDataElements) /
				 sizeof (struct cm_dm_data_element)),
				&(actionDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT (" group desc DataElement, Error : %d", return_value);
		return OF_FAILURE;
	}

	//Group Stats
	return_value =
		cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_GROUPS_STATS_APPL_ID, CM_DP_CHANNEL_ID,
				CM_DM_ON_DIRECTOR_DATAPATH_GROUPS_STATS_PATH,
				(sizeof (groupstatsDataElements) /
				 sizeof (struct cm_dm_data_element)),
				&(groupstatsDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT (" group features DataElement, Error : %d", return_value);
		return OF_FAILURE;
	}

	//Group Features
	return_value =
		cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_GROUPS_FEATURES_APPL_ID, CM_DP_CHANNEL_ID,
				CM_DM_ON_DIRECTOR_DATAPATH_GROUPS_FEATURES_PATH,
				(sizeof (groupfeaturesDataElements) /
				 sizeof (struct cm_dm_data_element)),
				&(groupfeaturesDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT (" group features DataElement, Error : %d", return_value);
		return OF_FAILURE;
	}

    //table feature
	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_DATAPATH_STATS_TABLEFEATURES_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_DATAPATH_STATS_TABLEFEATURES_PATH,
					 (sizeof (tablefeaturesDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(tablefeaturesDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
		 CM_CBK_DEBUG_PRINT (" tablefeaturesDataElements, Error : %d", return_value );
		 return OF_FAILURE;
	}

     //flow mod
	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_DATAPATH_FLOWMOD_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_DATAPATH_FLOWMOD_PATH,
					 (sizeof (flowmodDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(flowmodDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT (" flowmodDataElements, Error : %d", return_value );
           return OF_FAILURE;
	}

	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_DATAPATH_FLOWMOD_MATCHFIELD_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_DATAPATH_FLOWMOD_MATCHFIELD_PATH,
					 (sizeof (matchfieldDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(matchfieldDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT (" match field DataElements, Error : %d", return_value );
           return OF_FAILURE;
	}

	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_DATAPATH_FLOWMOD_INSTRUCTION_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_DATAPATH_FLOWMOD_INSTRUCTION_PATH,
					 (sizeof (instructionDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(instructionDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT (" instruction DataElements, Error : %d", return_value );
           return OF_FAILURE;
	}

	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_DATAPATH_FLOWMOD_INSTRUCTION_ACTIONLIST_APPL_ID,
                                              CM_DP_CHANNEL_ID,
					      CM_DM_ON_DIRECTOR_DATAPATH_FLOWMOD_INSTRUCTION_ACTIONLIST_PATH,
					      (sizeof (actionlistDataElements) /
                                               sizeof (struct cm_dm_data_element)),
					      &(actionlistDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT ("action list DataElements, Error : %d", return_value );
           return OF_FAILURE;
	}

	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_DATAPATH_FLOWMOD_ACTIONSET_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_DATAPATH_FLOWMOD_ACTIONSET_PATH,
					 (sizeof (actionsetDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(actionsetDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT ("action set DataElements, Error : %d", return_value );
           return OF_FAILURE;
	}

	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_SSL_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_SSL_PATH,
					 (sizeof (sslDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(sslDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT ("SSL set DataElements, Error : %d", return_value );
           return OF_FAILURE;
	}

	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_SSL_ADNLCA_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_SSL_ADNLCA_PATH,
					 (sizeof (adnlcaDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(adnlcaDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT ("action set additional DataElements, Error : %d", return_value );
           return OF_FAILURE;
	}


        /** CRM DATA Models **/
	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_CRM_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_CRM_PATH,
					 (sizeof (crmDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(crmDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT ("crm  reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
	}

	


        /* tenant reg */
	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_CRM_TENANT_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_CRM_TENANT_PATH,
					 (sizeof (tenantDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(tenantDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT ("crm Tenant reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
	}

        /* tenant attribute reg*/
	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_CRM_TENANT_TENANTATTRIBUTE_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_CRM_TENANT_TENANTATTRIBUTE_PATH,
					 (sizeof (tenantattributeDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(tenantattributeDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT ("crm tenantattributeDataElements reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
	}




        /* vm reg */
	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_CRM_VIRTUALMACHINE_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_CRM_VIRTUALMACHINE_PATH,
					 (sizeof (virtualmachineDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(virtualmachineDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT ("crm vm reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
	}

#if 0
	/* vmports to vm datamodel reg*/
	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_CRM_VIRTUALMACHINE_VMPORTS_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_CRM_VIRTUALMACHINE_VMPORTS_PATH,
					 (sizeof (vmportsDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(vmportsDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT ("crm vmportsDataElements reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
	}
#endif
	/* vm attribute reg */
	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_CRM_VIRTUALMACHINE_VMATTRIBUTE_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_CRM_VIRTUALMACHINE_VMATTRIBUTE_PATH,
					 (sizeof (vmattributeDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(vmattributeDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT ("crm vmattributeDataElements DataElements, Error : %d", return_value );
           return OF_FAILURE;
	}



       /* subnet reg */
	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_CRM_SUBNET_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_CRM_SUBNET_PATH,
					 (sizeof (subnetDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(subnetDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT ("crm subnetDataElements reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
	}


	/* subnet attribute reg */
	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_CRM_SUBNET_SUBNETATTRIBUTE_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_CRM_SUBNET_SUBNETATTRIBUTE_PATH,
					 (sizeof (subnetattributeDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(subnetattributeDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT ("crm subnetattributeDataElements DataElements, Error : %d", return_value );
           return OF_FAILURE;
	}



       /* vn reg */
	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_CRM_VIRTUALNETWORK_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_CRM_VIRTUALNETWORK_PATH,
					 (sizeof (virtualnetworkDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(virtualnetworkDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT ("crm virtualnetworkDataElements reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
	}
	/* vn attribute datamodel reg*/
	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_CRM_VIRTUALNETWORK_VNATTRIBUTE_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_CRM_VIRTUALNETWORK_VNATTRIBUTE_PATH,
					 (sizeof (vnattributeDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(vnattributeDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT ("crm vnattributeDataElements reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
	}


	/* computenodes datamodel reg*/
	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_CRM_VIRTUALNETWORK_COMPUTENODES_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_CRM_VIRTUALNETWORK_COMPUTENODES_PATH,
					 (sizeof (computenodesDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(computenodesDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT ("crm computenodesDataElements reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
	}

	/* vmside ports to vn datamodel reg*/
	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_CRM_VIRTUALNETWORK_COMPUTENODES_VMSIDEPORTS_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_CRM_VIRTUALNETWORK_COMPUTENODES_VMSIDEPORTS_PATH,
					 (sizeof (vmsideportsDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(vmsideportsDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT ("crm vmsideportsDataElements reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
	}
    
        return_value =
                 cm_dm_register_data_elements(CM_ON_DIRECTOR_CRM_VIRTUALNETWORK_COMPUTENODES_VMSIDEPORTS_PORTATTR_APPL_ID, CM_DP_CHANNEL_ID,
                                         CM_DM_ON_DIRECTOR_CRM_VIRTUALNETWORK_COMPUTENODES_VMSIDEPORTS_PORTATTR_PATH,
                                         (sizeof (portattrDataElements) /
                                                sizeof (struct cm_dm_data_element)),
                                         &(portattrDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
           CM_CBK_DEBUG_PRINT ("crm vmsideportsDataElements reg DataElements,Error : %d",return_value);
           return OF_FAILURE;
        }

	/* nwside ports to  datamodel reg*/
	return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_CRM_NWSIDEPORTS_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_CRM_NWSIDEPORTS_PATH,
					 (sizeof (nwsideportsDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(nwsideportsDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT ("crm vmsideportsDataElements reg DataElements,Error : %d", return_value );
           return OF_FAILURE;
	}
        return_value =
		 cm_dm_register_data_elements(CM_ON_DIRECTOR_CRM_NWSIDEPORTS_PORTATTRIB_APPL_ID, CM_DP_CHANNEL_ID,
					 CM_DM_ON_DIRECTOR_CRM_NWSIDEPORTS_PORTATTRIB_PATH,
					 (sizeof (nwportattrDataElements) /
						sizeof (struct cm_dm_data_element)),
					 &(nwportattrDataElements[0]));
	if (return_value != OF_SUCCESS)
	{
           CM_CBK_DEBUG_PRINT ("crm vmsideportsDataElements reg DataElements,Error : %d", return_value );
           return OF_FAILURE;
	}


        /*NSRM Datamodel reg*/
        return_value =
                 cm_dm_register_data_elements(CM_ON_DIRECTOR_NSRM_APPL_ID, CM_DP_CHANNEL_ID,
                                         CM_DM_ON_DIRECTOR_NSRM_PATH,
                                         (sizeof (nsrmDataElements) /
                                                sizeof (struct cm_dm_data_element)),
                                         &(nsrmDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
           CM_CBK_DEBUG_PRINT ("nsrm reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
        }

       return_value = cm_dm_register_data_elements(CM_ON_DIRECTOR_NSRM_SERVICE_APPL_ID, CM_DP_CHANNEL_ID,
		CM_DM_ON_DIRECTOR_NSRM_SERVICE_PATH,
		(sizeof (serviceDataElements) /
		 sizeof (struct cm_dm_data_element)),
		&(serviceDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
          CM_CBK_DEBUG_PRINT ("nsrm appliance reg DataElements, Error : %d", return_value );
          return OF_FAILURE;
        }
 
       return_value = cm_dm_register_data_elements(CM_ON_DIRECTOR_NSRM_SERVICE_SERVICEATTRIBUTE_APPL_ID,                                                    CM_DP_CHANNEL_ID,
	                                       	   CM_DM_ON_DIRECTOR_NSRM_SERVICE_SERVICEATTRIBUTE_PATH, 
                                 		   (sizeof (serviceattributeDataElements) /
		                                   sizeof (struct cm_dm_data_element)),
		                                   &(serviceattributeDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
          CM_CBK_DEBUG_PRINT ("nsrm appliance reg DataElements, Error : %d", return_value );
          return OF_FAILURE;
        }

        return_value =
                 cm_dm_register_data_elements(CM_ON_DIRECTOR_NSRM_CHAIN_APPL_ID, CM_DP_CHANNEL_ID,
                                         CM_DM_ON_DIRECTOR_NSRM_CHAIN_PATH,
                                         (sizeof (chainDataElements) /
                                                sizeof (struct cm_dm_data_element)),
                                         &(chainDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
           CM_CBK_DEBUG_PRINT ("nsrm ns chain reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
        }
        
        return_value = cm_dm_register_data_elements(CM_ON_DIRECTOR_NSRM_CHAIN_CHAINATTRIBUTE_APPL_ID,                                                        CM_DP_CHANNEL_ID,
	                                       	   CM_DM_ON_DIRECTOR_NSRM_CHAIN_CHAINATTRIBUTE_PATH, 
                                 		   (sizeof (chainattributeDataElements) /
		                                   sizeof (struct cm_dm_data_element)),
		                                   &(chainattributeDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
          CM_CBK_DEBUG_PRINT ("nsrm appliance reg DataElements, Error : %d", return_value );
          return OF_FAILURE;
        }
        return_value =
                 cm_dm_register_data_elements(CM_ON_DIRECTOR_NSRM_CHAINSET_APPL_ID, CM_DP_CHANNEL_ID,
                                         CM_DM_ON_DIRECTOR_NSRM_CHAINSET_PATH,
                                         (sizeof (chainsetDataElements) /
                                                sizeof (struct cm_dm_data_element)),
                                         &(chainsetDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
           CM_CBK_DEBUG_PRINT ("nsrm ns chainset reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
        }
        return_value = cm_dm_register_data_elements(CM_ON_DIRECTOR_NSRM_CHAINSET_CHAINSETATTRIBUTE_APPL_ID,                                                        CM_DP_CHANNEL_ID,
	                                       	   CM_DM_ON_DIRECTOR_NSRM_CHAINSET_CHAINSETATTRIBUTE_PATH, 
                                 		   (sizeof (chainsetattributeDataElements) /
		                                   sizeof (struct cm_dm_data_element)),
		                                   &(chainsetattributeDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
          CM_CBK_DEBUG_PRINT ("nsrm appliance reg DataElements, Error : %d", return_value );
          return OF_FAILURE;
        }

        return_value =
                 cm_dm_register_data_elements(CM_ON_DIRECTOR_NSRM_NETWORKMAP_APPL_ID, CM_DP_CHANNEL_ID,
                                         CM_DM_ON_DIRECTOR_NSRM_NETWORKMAP_PATH,
                                         (sizeof (networkmapDataElements) /
                                                sizeof (struct cm_dm_data_element)),
                                         &(networkmapDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
           CM_CBK_DEBUG_PRINT ("nsrm ns chainset reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
        }
      
       return_value = cm_dm_register_data_elements(CM_ON_DIRECTOR_NSRM_NETWORKMAP_L2NETWORKMAPATTRIBUTE_APPL_ID,                                                        CM_DP_CHANNEL_ID,
	                                       	   CM_DM_ON_DIRECTOR_NSRM_NETWORKMAP_L2NETWORKMAPATTRIBUTE_PATH, 
                                 		   (sizeof (l2networkmapattributeDataElements) /
		                                   sizeof (struct cm_dm_data_element)),
		                                   &(l2networkmapattributeDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
          CM_CBK_DEBUG_PRINT ("nsrm appliance reg DataElements, Error : %d", return_value );
          return OF_FAILURE;
        }

       return_value =
                 cm_dm_register_data_elements(CM_ON_DIRECTOR_NSRM_CHAINSET_CHAINSELRULE_APPL_ID, CM_DP_CHANNEL_ID,
                                         CM_DM_ON_DIRECTOR_NSRM_CHAINSET_CHAINSELRULE_PATH,
                                         (sizeof (chainselruleDataElements) /
                                                sizeof (struct cm_dm_data_element)),
                                         &(chainselruleDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
           CM_CBK_DEBUG_PRINT ("nsrm ns chain selection rule reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
        }



        return_value =
                 cm_dm_register_data_elements(CM_ON_DIRECTOR_NSRM_CHAINSET_CHAINSETZONERULE_APPL_ID, CM_DP_CHANNEL_ID,
                                         CM_DM_ON_DIRECTOR_NSRM_CHAINSET_CHAINSETZONERULE_PATH,
                                         (sizeof (chainsetzoneruleDataElements) /
                                                sizeof (struct cm_dm_data_element)),
                                         &(chainsetzoneruleDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
           CM_CBK_DEBUG_PRINT ("nsrm zone rule reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
        }

       return_value =
                 cm_dm_register_data_elements(CM_ON_DIRECTOR_NSRM_CHAINSET_CHAINSELRULE_CHAINSELECTIONATTRIBUTE_APPL_ID, CM_DP_CHANNEL_ID,
                                         CM_DM_ON_DIRECTOR_NSRM_CHAINSET_CHAINSELRULE_CHAINSELECTIONATTRIBUTE_PATH,
                                         (sizeof (chainselectionattributeDataElements) /
                                                sizeof (struct cm_dm_data_element)),
                                         &(chainselectionattributeDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
           CM_CBK_DEBUG_PRINT ("nsrm ns chain selection rule reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
        }




        return_value =
                 cm_dm_register_data_elements(CM_ON_DIRECTOR_NSRM_CHAIN_BYPASSRULE_APPL_ID, CM_DP_CHANNEL_ID,
                                         CM_DM_ON_DIRECTOR_NSRM_CHAIN_BYPASSRULE_PATH,
                                         (sizeof (bypassruleDataElements) /
                                                sizeof (struct cm_dm_data_element)),
                                         &(bypassruleDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
           CM_CBK_DEBUG_PRINT ("nsrm ns chain selection rule reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
        }

       return_value =
                 cm_dm_register_data_elements(CM_ON_DIRECTOR_NSRM_CHAIN_BYPASSRULE_BYPASSATTRIBUTE_APPL_ID, CM_DP_CHANNEL_ID,
                                         CM_DM_ON_DIRECTOR_NSRM_CHAIN_BYPASSRULE_BYPASSATTRIBUTE_PATH,
                                         (sizeof (bypassattributeDataElements) /
                                                sizeof (struct cm_dm_data_element)),
                                         &(bypassattributeDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
           CM_CBK_DEBUG_PRINT ("nsrm ns chain selection rule reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
        }



       return_value =
                 cm_dm_register_data_elements(CM_DM_BYPASSRULE_BYPASS_SERVICES_ID, CM_DP_CHANNEL_ID,
                                         CM_DM_ON_DIRECTOR_NSRM_CHAIN_BYPASSRULE_BYPASS_SERVICES_PATH,
                                         (sizeof (servicenamesDataElements) /
                                                sizeof (struct cm_dm_data_element)),
                                         &(servicenamesDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
           CM_CBK_DEBUG_PRINT ("nsrm ns chain selection rule reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
        }

       
       return_value =
                 cm_dm_register_data_elements(CM_ON_DIRECTOR_NSRM_CHAIN_SERVICES_APPL_ID, CM_DP_CHANNEL_ID,
                                         CM_DM_ON_DIRECTOR_NSRM_CHAIN_SERVICES_PATH,
                                         (sizeof (servicesDataElements) /
                                                sizeof (struct cm_dm_data_element)),
                                         &(servicesDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
           CM_CBK_DEBUG_PRINT ("nsrm ns chain selection rule reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
        }
       

       return_value =
                 cm_dm_register_data_elements(CM_ON_DIRECTOR_NSRM_CHAIN_SERVICES_SERVICE_INSTANCE_APPL_ID, CM_DP_CHANNEL_ID,
                                         CM_DM_ON_DIRECTOR_NSRM_CHAIN_SERVICES_SERVICE_INSTANCE_PATH,
                                         (sizeof (service_instanceDataElements) /
                                                sizeof (struct cm_dm_data_element)),
                                         &(service_instanceDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
           CM_CBK_DEBUG_PRINT ("nsrm ns chain selection rule reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
        }

 /* TTP registration */
       return_value = cm_dm_register_data_elements(CM_ON_DIRECTOR_TTP_APPL_ID, 
                           CM_DP_CHANNEL_ID, CM_DM_ON_DIRECTOR_TTP_PATH,
                           (sizeof (ttpDataElements) /
                                sizeof (struct cm_dm_data_element)),
                            &(ttpDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
           CM_CBK_DEBUG_PRINT ("ttp reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
        }

       return_value = cm_dm_register_data_elements(CM_ON_DIRECTOR_TTP_OFTABLES_APPL_ID, 
                           CM_DP_CHANNEL_ID, CM_DM_ON_DIRECTOR_TTP_OFTABLES_PATH,
                           (sizeof (oftablesDataElements) /
                                sizeof (struct cm_dm_data_element)),
                            &(oftablesDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
           CM_CBK_DEBUG_PRINT ("ttp reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
        }

       return_value = cm_dm_register_data_elements(CM_ON_DIRECTOR_TTP_DOMAINS_APPL_ID, 
                           CM_DP_CHANNEL_ID, CM_DM_ON_DIRECTOR_TTP_DOMAINS_PATH,
                           (sizeof (domainsDataElements) /
                                sizeof (struct cm_dm_data_element)),
                            &(domainsDataElements[0]));
        if (return_value != OF_SUCCESS)
        {
           CM_CBK_DEBUG_PRINT ("ttp reg DataElements, Error : %d", return_value );
           return OF_FAILURE;
        }


  /* End TTP registration */

       /*que config */
  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_QUEUE_CONFIG_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_QUEUE_CONFIG_PATH,
        (sizeof (queue_configDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(queue_configDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Queue Config data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

/*que config */

/* SUB TTP Stats */
  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_SUBTTP_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_SUBTTP_PATH,
        (sizeof (subttpDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(subttpDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("SUB TTP stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_SUBTTP_ARP_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_SUBTTP_ARP_PATH,
        (sizeof (subttparpDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(subttparpDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("SUB TTP stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_SUBTTP_ARP_STATS_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_SUBTTP_ARP_STATS_PATH,
        (sizeof (arpsubttpstatsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(arpsubttpstatsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("SUB TTP stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_SUBTTP_ARP_STATS_ARPSTATS_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_SUBTTP_ARP_STATS_ARPSTATS_PATH,
        (sizeof (arpttpstatsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(arpttpstatsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("SUB TTP stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

  /* SUB TTP Firewall*/
  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_SUBTTP_FIREWALL_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_SUBTTP_FIREWALL_PATH,
        (sizeof (ttpfirewallstatsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(ttpfirewallstatsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("SUB TTP stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_SUBTTP_FIREWALL_STATS_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_SUBTTP_FIREWALL_STATS_PATH,
        (sizeof (fwttpstatsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(fwttpstatsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("SUB TTP stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_SUBTTP_FIREWALL_STATS_FWCONSTATS_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_SUBTTP_FIREWALL_STATS_FWCONSTATS_PATH,
        (sizeof (fwconsubttpstatsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(fwconsubttpstatsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("SUB TTP stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_SUBTTP_FIREWALL_STATS_FWFWD_RULESTATS_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_SUBTTP_FIREWALL_STATS_FWFWD_RULESTATS_PATH,
        (sizeof (fwdsubttpstatsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(fwdsubttpstatsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("SUB TTP stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

  /* SUB TTP Route stats */
  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_SUBTTP_ROUTE_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_SUBTTP_ROUTE_PATH,
        (sizeof (routettpDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(routettpDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("SUB TTP stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_SUBTTP_ROUTE_STATS_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_SUBTTP_ROUTE_STATS_PATH,
        (sizeof (routettpstatsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(routettpstatsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("SUB TTP stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

  return_value =
    cm_dm_register_data_elements (CM_ON_DIRECTOR_DATAPATH_SUBTTP_ROUTE_STATS_ROUTESTATS_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_DATAPATH_SUBTTP_ROUTE_STATS_ROUTESTATS_PATH,
        (sizeof (routesubttpstatsDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(routesubttpstatsDataElements[0]));
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("SUB TTP stats data elements registration, Error : %d", return_value);
    return OF_FAILURE;
  }

/* SUB TTP Stats */


    return_value =
                     cm_dm_register_data_elements(CM_ON_DIRECTOR_TSC_APPL_ID, CM_DP_CHANNEL_ID,
                                                  CM_DM_ON_DIRECTOR_TSC_PATH,
                                                 (sizeof (tscDataElements) /
                                                  sizeof (struct cm_dm_data_element)),
                                                  &(tscDataElements[0]));
                     if (return_value != OF_SUCCESS)
                     {
                         CM_CBK_DEBUG_PRINT ("tsc reg DataElements, Error : %d", return_value );
                         return OF_FAILURE;
                     }

    return_value =
                    cm_dm_register_data_elements(CM_ON_DIRECTOR_TSC_SELECTORS_APPL_ID, CM_DP_CHANNEL_ID,
                                                 CM_DM_ON_DIRECTOR_TSC_SELECTORS_PATH,
        (sizeof (tsc_selectorDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(tsc_selectorDataElements[0]));
if (return_value != OF_SUCCESS)
{
    CM_CBK_DEBUG_PRINT ("tsc reg DataElements, Error : %d", return_value );
    return OF_FAILURE;
}


    return_value =
cm_dm_register_data_elements(CM_ON_DIRECTOR_TSC_REPOSITORY_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_TSC_REPOSITORY_PATH,
        (sizeof (nsc_repositoryDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(nsc_repositoryDataElements[0]));
if (return_value != OF_SUCCESS)
{
    CM_CBK_DEBUG_PRINT ("tsc reg DataElements, Error : %d", return_value );
    return OF_FAILURE;
}


    return_value =
cm_dm_register_data_elements(CM_ON_DIRECTOR_TSC_TABLE_3_REPOSITORY_APPL_ID, CM_DP_CHANNEL_ID,
        CM_DM_ON_DIRECTOR_TSC_TABLE_3_REPOSITORY_PATH,
        (sizeof (nsc_table_3_repositoryDataElements) /
         sizeof (struct cm_dm_data_element)),
        &(nsc_table_3_repositoryDataElements[0]));
if (return_value != OF_SUCCESS)
{
    CM_CBK_DEBUG_PRINT ("tsc reg DataElements, Error : %d", return_value );
    return OF_FAILURE;
}

    return OF_SUCCESS;
}


#endif /*OF_CM_SUPPORT */
