// Natural Sort algorithm for Javascript - Version 0.7 - Released under MIT license
// Author: Jim Palmer (based on chunking idea from Dave Koelle)
export function naturalSort(a: any, b: any): number {
  let re = /(^-?[0-9]+(\.?[0-9]*)[df]?e?[0-9]?$|^0x[0-9a-f]+$|[0-9]+)/gi;
  let sre = /(^[ ]*|[ ]*$)/g;
  // tslint:disable-next-line:max-line-length
  let dre = /(^([\w ]+,?[\w ]+)?[\w ]+,?[\w ]+\d+:\d+(:\d+)?[\w ]?|^\d{1,4}[\/\-]\d{1,4}[\/\-]\d{1,4}|^\w+, \w+ \d+, \d{4})/;
  let hre = /^0x[0-9a-f]+$/i;
  let ore = /^0/;

  // convert all to strings strip whitespace
  let x = a.replace(sre, '') || '';
  let y = b.replace(sre, '') || '';

  // chunk/tokenize
  let xN = x.replace(re, '\0$1\0').replace(/\0$/, '').replace(/^\0/, '').split('\0');
  let yN = y.replace(re, '\0$1\0').replace(/\0$/, '').replace(/^\0/, '').split('\0');

  // numeric, hex or date detection
  let xM: any = x.match(hre);
  let yM: any = y.match(hre);
  let xD = Number.parseInt(xM) || (xN.length !== 1 && x.match(dre) && Date.parse(x));
  let yD = Number.parseInt(yM) || xD && y.match(dre) && Date.parse(y) || null;
  let oFxNcL: any;
  let oFyNcL: any;

  // first try and sort Hex codes or Dates
  if (yD) {
    if (xD < yD) {
      return -1;
    }
    else if (xD > yD) {
      return 1;
    }
  }

    // natural sorting through split numeric strings and default strings
  for (let cLoc = 0, numS = Math.max(xN.length, yN.length); cLoc < numS; ++cLoc) {
    // find floats not starting with '0', string or 0 if not defined (Clint Priest)
    oFxNcL = !(xN[cLoc] || '').match(ore) && parseFloat(xN[cLoc]) || xN[cLoc] || 0;
    oFyNcL = !(yN[cLoc] || '').match(ore) && parseFloat(yN[cLoc]) || yN[cLoc] || 0;

    // handle numeric vs string comparison - number < string - (Kyle Adams)
    if (isNaN(oFxNcL) !== isNaN(oFyNcL)) {
      return (isNaN(oFxNcL)) ? 1 : -1;
      }
    // rely on string comparison if different types - i.e. '02' < 2 != '02' < '2'
    else if (typeof oFxNcL !== typeof oFyNcL) {
      oFxNcL += '';
      oFyNcL += '';
    }

    if (oFxNcL < oFyNcL) {
      return -1;
    }
    if (oFxNcL > oFyNcL) {
      return 1;
    }
  }

  return 0;
}
