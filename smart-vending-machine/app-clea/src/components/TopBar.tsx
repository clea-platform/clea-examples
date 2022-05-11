import React from "react";
import moment
 from "moment";
import { TransactionData, BeverageFromLongToShort } from "../types";
import { indexOf } from "lodash";

// -------- STYLE --------
const cardStyle = {
    borderRadius: "9px",
    height: "100%",
}
const titleStyle = {
    color: "#0631cc",
    fontSize: "29px",
    fontWeight: "700",
}
const subTitleStyle = {
    color: "#999999",
    fontSize: "14px",
    fontWeight: "550",
};
const textCardStyle = {
    color: "black",
    fontSize: "10px",
};
const suggestionCardStyle = {
    backgroundColor: "#11b0ef",
    borderRadius: "9px",
    height: "100%",
}
const suggestionTitleStyle = {
    color: "#ceeefb",
    fontSize: "14px",
    fontWeight: "700",
}
const suggestionBodyStyle = {
    color: "white",
    fontSize: "14px",
    fontWeight: "650",
}

const allBeverages = Object.keys(BeverageFromLongToShort)

type PageProps= {
    transactions: TransactionData[]
}

const TopBar: React.FC<PageProps> = ( { transactions } ) => {

    let revenue = 0;
    let count = 0;
    const arrayBestSellerCount: number[] = new Array(allBeverages.length).fill(0)
    const arrayBestSellerPrice: number[] = new Array(allBeverages.length).fill(0)

    transactions.forEach((t) => {
        // if is today
        if (moment.unix(t.timestamp).isSame(new Date(), "day"))
            revenue += t.price;
            count += 1
            if (allBeverages.includes(t.choice)) {
                arrayBestSellerCount[allBeverages.indexOf(t.choice)] += 1;
                arrayBestSellerPrice[allBeverages.indexOf(t.choice)] += t.price;
            }
    });

    const solded = Math.max(...arrayBestSellerCount);
    const revenueText = revenue.toFixed(2) + "€";
    const beverageSolded = count.toFixed(0) + " beverage sold";
    const bestSeller = allBeverages[arrayBestSellerCount.indexOf(solded)];
    const bestSellerSolded = solded.toFixed(0)  + " sold";
    const suggestionBody = "Espresso bin is filling up please check it";

    return (
        <>
            <div className="col px-0">
                <div className="card-custom" style={cardStyle}>
                    <div className="card-title text-center mt-1 mb-0" style={titleStyle}>
                        {revenueText}
                    </div>
                    <div className="card-subtitle text-center" style={subTitleStyle}>
                        Today's revenue
                    </div>
                    <div className="text-center mb-2" style={textCardStyle}>
                        {beverageSolded}
                    </div>
                </div>
            </div>
            <div className="col">
                <div className="card-custom" style={cardStyle}>
                    <div className="card-title text-center mt-1 mb-0" style={titleStyle}>
                        {bestSeller}
                    </div>
                    <div className="card-subtitle text-center" style={subTitleStyle}>
                        Best Seller
                    </div>
                    <div className="text-center mb-2" style={textCardStyle}>
                        {bestSellerSolded}
                    </div>
                </div>
            </div>
            <div className="col px-0">
                <div className="card-custom" style={suggestionCardStyle}>
                    <div className="card-title ms-3 mt-1 mb-0 pt-2" style={suggestionTitleStyle}>
                        Smart Suggestion
                    </div>
                    <div className="mx-3" style={suggestionBodyStyle}>
                        {suggestionBody}
                    </div>
                </div>
            </div>
        </>
    );
};

export default TopBar;