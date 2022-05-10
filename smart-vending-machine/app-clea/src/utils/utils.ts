export function pad(num: number, size:number) {
    let stringNum: string = num.toString();
    while (stringNum.length < size) stringNum = stringNum + "0";
    return stringNum;
}
