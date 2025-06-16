import { useCallback, useEffect, useState } from "react";
import { createRoot } from "react-dom/client";
import type { Root } from "react-dom/client";
import { FormattedMessage, IntlProvider } from "react-intl";
import Button from "react-bootstrap/Button";
import FormSelect from "react-bootstrap/FormSelect";
import Spinner from "react-bootstrap/Spinner";
import Table from "react-bootstrap/Table";
import {
  ApolloClient,
  InMemoryCache,
  ApolloProvider,
  gql,
  useQuery,
  useMutation,
  createHttpLink,
} from "@apollo/client";
import { setContext } from "@apollo/client/link/context";

import en from "./i18n/langs-compiled/en.json";
import it from "./i18n/langs-compiled/it.json";

const messages = { en, it };

const GET_APPLIANCES = gql`
  query GetAppliances($first: Int!, $after: String, $filter: ApplianceFilter) {
    appliances(first: $first, after: $after, filter: $filter)
      @connection(key: "appliances", filter: ["filter"]) {
      edges {
        node {
          id
          name
          serial
          tags
          device {
            deviceId
            astarteRealm
            baseApiUrl
          }
        }
      }
      pageInfo {
        hasNextPage
      }
    }
    existingApplianceTags
  }
`;

const GENERATE_ASTARTE_TOKEN = gql`
  mutation GenerateAstarteToken($input: GenerateAstarteTokenInput!) {
    generateAstarteToken(input: $input) {
      astarteToken
    }
  }
`;

const useAstarteToken = (applianceId: string): string | null => {
  const [generateAstarteToken, { data }] = useMutation(GENERATE_ASTARTE_TOKEN);

  const astarteToken = data?.generateAstarteToken?.astarteToken || null;
  const expiresAt = data?.generateAstarteToken?.expiresAt || null;

  useEffect(() => {
    if (!expiresAt || new Date(expiresAt) <= new Date())
      generateAstarteToken({ variables: { input: { applianceId } } });
  }, [applianceId, expiresAt]);

  return astarteToken;
};

type ConnectionStatus = {
  connected: boolean;
  lastConnection: string | null;
  lastDisconnection: string | null;
};

const useDeviceConnectionStatus = (
  deviceId: string,
  astarteRealm: string,
  astarteBaseApiURL: string,
  authToken: string | null,
) => {
  const [status, setStatus] = useState<ConnectionStatus | null>(null);

  const getStatus = useCallback(() => {
    let mounted = true;
    if (!authToken) {
      return;
    }
    const deviceStatusUrl = new URL(
      `/appengine/v1/${astarteRealm}/devices/${deviceId}`,
      astarteBaseApiURL,
    );
    fetch(deviceStatusUrl, {
      headers: {
        Authorization: `Bearer ${authToken}`,
      },
    })
      .then((response) => response.json())
      .then(
        (response) =>
          mounted &&
          setStatus({
            connected: response.data.connected || false,
            lastConnection: response.data.last_connection || null,
            lastDisconnection: response.data.last_disconnection || null,
          }),
      )
      .catch(() => mounted && setStatus(null));
    return () => {
      mounted = false;
    };
  }, [astarteBaseApiURL, authToken, deviceId, astarteRealm]);

  useEffect(() => {
    getStatus();
    const intervalId = setInterval(getStatus, 10000);
    return () => clearInterval(intervalId);
  }, [getStatus]);

  return status;
};

type DeviceConnectionStatusProps = {
  appliance: {
    id: string;
    device: {
      deviceId: string;
      astarteRealm: string;
      baseApiUrl: string;
    };
  };
};

const DeviceConnectionStatus = ({ appliance }: DeviceConnectionStatusProps) => {
  const token = useAstarteToken(appliance.id);
  const status = useDeviceConnectionStatus(
    appliance.device.deviceId,
    appliance.device.astarteRealm,
    appliance.device.baseApiUrl,
    token,
  );
  return status?.connected ? (
    <FormattedMessage
      id="appliance_status_connected"
      defaultMessage="Connected"
    />
  ) : (
    <FormattedMessage
      id="appliance_status_disconnected"
      defaultMessage="Disconnected"
    />
  );
};

const PAGE_SIZE = 2;

type AppProps = {
  clea: {
    tenantApiUrl: string;
    authToken: string;
  };
};

const App = (_props: AppProps) => {
  const [requestedPageCount, setRequestedPageCount] = useState(1);
  const [tagFilter, setTagFilter] = useState("");

  const appliancesQuery = useQuery(GET_APPLIANCES, {
    variables: {
      first: requestedPageCount * PAGE_SIZE,
      filter: tagFilter ? { tag: tagFilter } : undefined,
    },
  });

  useEffect(() => {
    setRequestedPageCount(1);
    const filter = tagFilter ? { tag: tagFilter } : undefined;
    appliancesQuery.refetch({ first: PAGE_SIZE, filter });
  }, [appliancesQuery.refetch, tagFilter]);

  return (
    <div>
      <h4>
        <FormattedMessage
          id="application_title"
          defaultMessage="Search appliances by tag"
        />
      </h4>
      <div className="mt-3">
        <FormSelect onChange={(event) => setTagFilter(event.target.value)}>
          <option value="">
            <FormattedMessage
              id="all_appliances_selection"
              defaultMessage="All appliances"
            />
          </option>
          {appliancesQuery.data?.existingApplianceTags.map((tag: string) => (
            <option key={tag} value={tag}>
              {tag}
            </option>
          ))}
        </FormSelect>
        <Table>
          <thead>
            <tr>
              <th>
                <FormattedMessage
                  id="appliance_name"
                  defaultMessage="Appliance Name"
                />
              </th>
              <th>
                <FormattedMessage id="appliance_serial" defaultMessage="S/N" />
              </th>
              <th>
                <FormattedMessage id="appliance_tags" defaultMessage="Tags" />
              </th>
              <th>
                <FormattedMessage
                  id="appliance_connection_status"
                  defaultMessage="Connection Status"
                />
              </th>
            </tr>
          </thead>
          <tbody>
            {appliancesQuery.data?.appliances.edges.map((edge: any) => (
              <tr key={edge.node.id}>
                <td>{edge.node.name}</td>
                <td>{edge.node.serial}</td>
                <td>{edge.node.tags.join(", ")}</td>
                <td>
                  <DeviceConnectionStatus appliance={edge.node} />
                </td>
              </tr>
            ))}
            {appliancesQuery.loading && (
              <tr>
                <td colSpan={4} className="text-center">
                  <Spinner />
                </td>
              </tr>
            )}
            {appliancesQuery.error && (
              <tr>
                <td colSpan={4} className="text-center">
                  <p>
                    <FormattedMessage
                      id="appliance_loading_error"
                      defaultMessage="Failed to load the appliance list."
                    />
                  </p>
                  <p>{appliancesQuery.error.message}</p>
                  <Button onClick={() => appliancesQuery.refetch()}>
                    <FormattedMessage
                      id="try_again_button"
                      defaultMessage="Try again"
                    />
                  </Button>
                </td>
              </tr>
            )}
          </tbody>
        </Table>
        {appliancesQuery.data?.appliances.pageInfo.hasNextPage && (
          <Button
            disabled={appliancesQuery.loading}
            onClick={() => {
              appliancesQuery.fetchMore({
                variables: {
                  first: (requestedPageCount + 1) * PAGE_SIZE,
                },
              });
              setRequestedPageCount(requestedPageCount + 1);
            }}
          >
            <FormattedMessage
              id="load_more_appliances_button"
              defaultMessage="Load more"
            />
          </Button>
        )}
      </div>
    </div>
  );
};

type UserPreferences = {
  language: "en" | "it";
};

type Settings = {
  themeUrl: string;
  userPreferences: UserPreferences;
};

let root: Root | null = null;

const AppLifecycle = {
  mount: (container: ShadowRoot, appProps: AppProps, settings: Settings) => {
    const { themeUrl, userPreferences } = settings;
    const { language } = userPreferences;

    const httpLink = createHttpLink({
      uri: appProps.clea.tenantApiUrl,
    });

    const authLink = setContext((_, { headers }) => {
      return {
        headers: {
          ...headers,
          authorization: "Bearer " + appProps.clea.authToken,
        },
      };
    });

    const client = new ApolloClient({
      link: authLink.concat(httpLink),
      cache: new InMemoryCache(),
    });

    root = createRoot(container);

    root.render(
      <>
        <link href={themeUrl} type="text/css" rel="stylesheet" />
        <IntlProvider
          messages={messages[language]}
          locale={language}
          defaultLocale="en"
        >
          <ApolloProvider client={client}>
            <App {...appProps} />
          </ApolloProvider>
        </IntlProvider>
      </>,
    );
  },
  unmount: (container: ShadowRoot) => root?.unmount(),
};

export default AppLifecycle;
