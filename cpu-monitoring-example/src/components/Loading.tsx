import { Spinner } from "react-bootstrap";
import { FormattedMessage } from "react-intl";

const Loading = () => (
  <div className="p-2 p-md-4 text-center">
    <Spinner
      animation="border"
      variant="primary"
      style={{ marginRight: "10px" }}
    />
    <FormattedMessage id="loading" defaultMessage="Loading..." />
  </div>
);
export default Loading;
