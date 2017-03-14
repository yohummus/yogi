export function timestampToDate(timestamp: number): Date {
  return new Date(timestamp / 1000000);
}

export function dateToTimestamp(date: Date): number {
  return date.getTime() * 1000000;
}

export function timestampToStringForHtmlInput(timestamp: number): string {
  let str = timestampToDate(timestamp).toISOString();
  return str.substring(0, str.length - 1);
}

function makeSubSecondString(timestamp: number, pad: boolean): string {
  let ms = (Math.floor(timestamp / 1000000) % 1000).toString();
  let us = (Math.floor(timestamp / 1000) % 1000).toString();
  let ns = (timestamp % 1000).toString();

  if (pad) {
    while (ms.length < 3) {
      ms = '0' + ms;
    }

    while (us.length < 3) {
      us = '0' + us;
    }

    while (ns.length < 3) {
      ns = '0' + ns;
    }
  }

  return ms + ' ms ' + us + ' us ' + ns + ' ns';
}

export function timestampToString(timestamp: number, fixedWidth = false): string {
  let date = timestampToDate(timestamp);
  let str = dateToString(date);
  str += ' ' + makeSubSecondString(timestamp, fixedWidth);
  return str;
}

export function timestampToTooltipString(timestamp: number): string {
  let date = timestampToDate(timestamp);
  let str = dateToString(date);
  str += '\n' + makeSubSecondString(timestamp, false);
  str += `\nRaw timestamp: ${timestamp}`;
  return str;
}

export function dateToString(date: Date): string {
  return date.toLocaleString('en-GB');
}

export function secondsToDurationString(totalSeconds: number): string {
  if (totalSeconds < 0) {
      totalSeconds = 0;
  }

  let days = (totalSeconds / 86400) | 0;
  let hours = (totalSeconds / 3600) % 24 | 0;
  let minutes = (totalSeconds / 60) % 60 | 0;
  let seconds = totalSeconds % 60 | 0;

  let str = '';
  if (days) {
      str += `${days} days `;
  }

  str += hours   < 10 ? `0${hours}:`   : `${hours}:`;
  str += minutes < 10 ? `0${minutes}:` : `${minutes}:`;
  str += seconds < 10 ? `0${seconds}`  : `${seconds}`;

  return str;
}
