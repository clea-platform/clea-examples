import axios, { AxiosInstance } from "axios";

type AstarteClientProps = {
    astarte_url: URL;
    realm: string;
    token: string;
};

type Config = AstarteClientProps & {
    app_engine_url: URL;
};

type QueryParameters = {
    device_id: string;
    since_after?: string;
    since?: Date;
    to?: Date;
    limit?: number;
};

type MachineStatus    = {
    container_status    : string,
    water_status        : string
};

type BeveragesCounters  = {
    short_coffe : number,
    long_cofee  : number
};

class AstarteClient {
    config: Config;
    COUNTERS_INTERFACE:string   = `ai.clea.examples.machine.Counters`;
    STATUS_INTRFACE:string      = `ai.clea.examples.machine.Status`;


    constructor({ astarte_url, realm, token }: AstarteClientProps) {
        this.config = {
            astarte_url,
            realm,
            token,
            app_engine_url: new URL("/", astarte_url),
        };
    }


    async performQuery (params:QueryParameters, interface_name:string, path?:string) {
        const {app_engine_url,
                realm,
                token}          = this.config;
        const final_path        = `appengine/v1/${realm}/devices/${params.device_id}/interfaces` +
                                    `/${interface_name}${path!==undefined ? "/"+path : ""}`;
        const request_url       = new URL(final_path, app_engine_url);
        const query: Record<string, string> = {};
        if (params.since_after) {
            query.sinceAfter    = params.since_after;
        }
        if (params.since) {
            query.since = params.since.toISOString();
        }
        if (params.to) {
            query.to    = params.to.toISOString();
        }
        if (params.limit) {
            query.limit = params.limit.toString();
        }
        request_url.search = new URLSearchParams(query).toString();
        
        //console.log (`Querying to ${request_url.toString()}`)
        return axios ({
            method  : "get",
            url     : request_url.toString(),
            headers : {
                "Authorization" : `Bearer ${token}`,
                "Content-Type"  : "application/json;charset=UTF-8",
            }
        }).then ((response) => {
            return response.data.data
        })
    }


    async get_current_status (device_id:string, since?:Date) {
        let status_response : MachineStatus         = await this.performQuery ({device_id, since},
                                                                                this.STATUS_INTRFACE)
        let counters_response : BeveragesCounters   = await this.performQuery ({device_id, since},
                                                                                this.COUNTERS_INTERFACE)

        return {...status_response, ...counters_response};
    }


    async get_container_status (params : QueryParameters) {
        return this.performQuery (params, this.STATUS_INTRFACE, 'containerStatus')
    }


    async get_water_status (params : QueryParameters) {
        return this.performQuery (params, this.STATUS_INTRFACE, 'waterStatus')
    }


    async get_short_coffee_status (params : QueryParameters) {
        return this.performQuery (params, this.COUNTERS_INTERFACE, 'shortCoffee')
    }
    
    
    async get_long_coffee_status (params : QueryParameters) {
        return this.performQuery (params, this.COUNTERS_INTERFACE, 'longCoffee')
    }
}

export default AstarteClient;