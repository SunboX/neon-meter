import { mkdir, readFile, writeFile } from "node:fs/promises";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const scriptDir = dirname(fileURLToPath(import.meta.url));
const rootDir = join(scriptDir, "..");
const assetsDir = join(rootDir, "docs", "assets");

/** Reads a UTF-8 project file relative to the repository root. */
async function readProjectFile(path) {
  return readFile(join(rootDir, path), "utf8");
}

/** Escapes user-visible text for safe SVG output. */
function escapeXml(value) {
  return String(value)
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;");
}

/** Parses theme color definitions from src/theme.h. */
function parseTheme(source) {
  const theme = {};
  const pattern = /#define\s+(kTheme[A-Za-z0-9_]+)\s+lv_color_hex\(0x([0-9a-fA-F]{6})\)/g;
  for (const match of source.matchAll(pattern)) {
    theme[match[1]] = `#${match[2].toLowerCase()}`;
  }
  return theme;
}

/** Evaluates simple constexpr arithmetic after replacing known constants. */
function evaluateConstExpression(expression, constants) {
  let resolved = expression.trim();
  for (let pass = 0; pass < 8; pass += 1) {
    const previous = resolved;
    for (const [name, value] of Object.entries(constants)) {
      resolved = resolved.replace(new RegExp(`\\b${name}\\b`, "g"), String(value));
    }
    if (resolved === previous) break;
  }
  if (!/^[0-9+\-*/%().\s]+$/.test(resolved)) return undefined;
  return Function(`"use strict"; return (${resolved});`)();
}

/** Parses integer and boolean constexpr values from a C++ header. */
function parseConstants(source, existing = {}) {
  const constants = { ...existing };
  const pattern = /(?:static\s+)?constexpr\s+(?:int|uint8_t|bool)\s+(k[A-Za-z0-9_]+)\s*=\s*([^;]+);/g;
  for (const match of source.matchAll(pattern)) {
    const [, name, expression] = match;
    if (expression.trim() === "true" || expression.trim() === "false") {
      constants[name] = expression.trim() === "true";
      continue;
    }
    const value = evaluateConstExpression(expression, constants);
    if (value !== undefined) constants[name] = value;
  }
  return constants;
}

/** Returns the remaining-capacity percentage displayed by the firmware gauge. */
function gaugeRemainingPercent(percent) {
  return Math.max(0, Math.min(100, Math.round(100 - percent)));
}

/** Returns the usage-bar color used by the firmware gauge threshold rules. */
function gaugeColor(percent, theme) {
  const remaining = gaugeRemainingPercent(percent);
  if (remaining <= 20) return theme.previewHighBar;
  if (remaining <= 50) return theme.kThemeAmber;
  return theme.kThemeGreen;
}

/** Creates an SVG element string from attributes and optional content. */
function el(name, attrs = {}, content = "") {
  const attributes = Object.entries(attrs)
    .filter(([, value]) => value !== undefined && value !== null && value !== false)
    .map(([key, value]) => `${key}="${escapeXml(value)}"`)
    .join(" ");
  return `<${name}${attributes ? ` ${attributes}` : ""}>${content}</${name}>`;
}

/** Creates a self-closing SVG element string from attributes. */
function single(name, attrs = {}) {
  const attributes = Object.entries(attrs)
    .filter(([, value]) => value !== undefined && value !== null && value !== false)
    .map(([key, value]) => `${key}="${escapeXml(value)}"`)
    .join(" ");
  return `<${name}${attributes ? ` ${attributes}` : ""}/>`;
}

/** Converts polar arc coordinates to an SVG path. */
function describeArc(cx, cy, radius, startAngle, endAngle) {
  const start = polarToCartesian(cx, cy, radius, startAngle);
  const end = polarToCartesian(cx, cy, radius, endAngle);
  const largeArc = endAngle - startAngle <= 180 ? "0" : "1";
  return `M ${start.x.toFixed(2)} ${start.y.toFixed(2)} A ${radius} ${radius} 0 ${largeArc} 1 ${end.x.toFixed(2)} ${end.y.toFixed(2)}`;
}

/** Converts degrees into SVG cartesian coordinates. */
function polarToCartesian(cx, cy, radius, angleInDegrees) {
  const angleInRadians = (angleInDegrees * Math.PI) / 180;
  return {
    x: cx + radius * Math.cos(angleInRadians),
    y: cy + radius * Math.sin(angleInRadians),
  };
}

/** Returns README preview colors calibrated to the photographed CoreS3 display. */
function previewTheme(theme) {
  return {
    ...theme,
    previewBg: theme.kThemeBg,
    previewPanel: theme.kThemePanel,
    previewUsagePanel: theme.kThemeUsagePanel,
    previewPanelStroke: theme.kThemeAccent,
    previewBarTrack: theme.kThemeBarBg,
    previewDim: "#31eaff",
    previewHighBar: "#ffb8c4",
    previewText: "#effcff",
  };
}

/** Draws the shared compact Neon Meter mark. */
function brandMark(layout, theme) {
  const x = layout.kMargin;
  const y = layout.kTitleY + layout.kBrandMarkOffsetY;
  const gaugeX = x + layout.kBrandMarkGaugeX;
  const gaugeY = y + layout.kBrandMarkGaugeY;
  const radius = layout.kBrandMarkGaugeSize / 2;
  const cx = gaugeX + radius;
  const cy = gaugeY + radius;
  const needleCx = x + layout.kBrandMarkNeedleX + layout.kBrandMarkNeedleWidth / 2;
  const needleCy = y + layout.kBrandMarkNeedleY + layout.kBrandMarkNeedleHeight / 2;

  return el(
    "g",
    {},
    [
      single("rect", {
        x,
        y,
        width: layout.kBrandMarkWidth,
        height: layout.kBrandMarkHeight,
        rx: 6,
        fill: theme.kThemeBg2,
        filter: "url(#cyanGlow)",
      }),
      single("path", {
        d: describeArc(cx, cy, radius - 2, layout.kBrandMarkArcStartAngle, layout.kBrandMarkArcEndAngle),
        stroke: theme.kThemeIconTrack,
        "stroke-width": layout.kBrandMarkArcWidth,
        "stroke-linecap": "round",
        fill: "none",
      }),
      single("path", {
        d: describeArc(cx, cy, radius - 4, layout.kBrandMarkArcStartAngle, layout.kBrandMarkAccentArcEndAngle),
        stroke: theme.kThemeAccent,
        "stroke-width": layout.kBrandMarkAccentArcWidth,
        "stroke-linecap": "round",
        fill: "none",
      }),
      single("rect", {
        x: x + layout.kBrandMarkBaseX,
        y: y + layout.kBrandMarkBaseY,
        width: layout.kBrandMarkBaseWidth,
        height: layout.kBrandMarkBaseHeight,
        rx: 1,
        fill: theme.kThemeIconBase,
      }),
      single("rect", {
        x: x + layout.kBrandMarkNeedleX,
        y: y + layout.kBrandMarkNeedleY,
        width: layout.kBrandMarkNeedleWidth,
        height: layout.kBrandMarkNeedleHeight,
        rx: 2,
        fill: theme.kThemeAccent,
        transform: `rotate(${layout.kBrandMarkNeedleRotation / 10} ${needleCx} ${needleCy})`,
        filter: "url(#cyanGlow)",
      }),
      single("circle", {
        cx: x + layout.kBrandMarkHubX + layout.kBrandMarkHubSize / 2,
        cy: y + layout.kBrandMarkHubY + layout.kBrandMarkHubSize / 2,
        r: layout.kBrandMarkHubSize / 2,
        fill: theme.kThemeAccent,
      }),
      single("circle", {
        cx: x + layout.kBrandMarkHubX + layout.kBrandMarkHubSize / 2,
        cy: y + layout.kBrandMarkHubY + layout.kBrandMarkHubSize / 2,
        r: layout.kBrandMarkHubCoreSize / 2,
        fill: theme.kThemeBg2,
      }),
      single("circle", {
        cx: x + layout.kBrandMarkNodeX + layout.kBrandMarkNodeSize / 2,
        cy: y + layout.kBrandMarkNodeY + layout.kBrandMarkNodeSize / 2,
        r: layout.kBrandMarkNodeSize / 2,
        fill: theme.kThemeOrange,
      }),
      single("circle", {
        cx: x + layout.kBrandMarkLeftDotX + layout.kBrandMarkDotSize / 2,
        cy: y + layout.kBrandMarkDotY + layout.kBrandMarkDotSize / 2,
        r: layout.kBrandMarkDotSize / 2,
        fill: theme.kThemeIconCyanDot,
      }),
      single("circle", {
        cx: x + layout.kBrandMarkRightDotX + layout.kBrandMarkDotSize / 2,
        cy: y + layout.kBrandMarkDotY + layout.kBrandMarkDotSize / 2,
        r: layout.kBrandMarkDotSize / 2,
        fill: theme.kThemeIconOrangeDot,
      }),
    ].join("\n")
  );
}

/** Draws the shared header accent and battery label. */
function sharedHeader(layout, theme, battery = "100%") {
  return [
    brandMark(layout, theme),
    single("rect", {
      x: layout.kMargin,
      y: 44,
      width: layout.kScreenWidth - 2 * layout.kMargin,
      height: 2,
      rx: 1,
      fill: theme.kThemeAccent,
      opacity: 0.95,
      filter: "url(#cyanGlow)",
    }),
    text(layout.kScreenWidth - layout.kMargin, layout.kTitleY + 23, battery, {
      anchor: "end",
      size: 16,
      weight: 600,
      fill: theme.kThemeAccent,
    }),
  ].join("\n");
}

/** Draws an SVG text element with firmware-like defaults. */
function text(x, y, value, options = {}) {
  return el("text", {
    x,
    y,
    fill: options.fill,
    "font-size": options.size ?? 14,
    "font-weight": options.weight ?? 500,
    "text-anchor": options.anchor ?? "start",
    "dominant-baseline": options.baseline ?? "alphabetic",
    opacity: options.opacity,
  }, escapeXml(value));
}

/** Draws one rounded usage panel with progress state. */
function usagePanel(layout, theme, y, data) {
  const panelX = layout.kMargin;
  const barX = panelX + layout.kPanelBarX;
  const barY = y + layout.kPanelBarY + 1;
  const barWidth = layout.kPanelBarWidth;
  const remaining = gaugeRemainingPercent(data.percent);
  const fillWidth = remaining === 0 ? 0 : Math.max(8, Math.round((barWidth * remaining) / 100));
  const color = gaugeColor(data.percent, theme);

  return el(
    "g",
    {},
    [
      single("rect", {
        x: panelX,
        y,
        width: layout.kPanelWidth,
        height: layout.kPanelHeight,
        rx: 6,
        fill: theme.previewUsagePanel,
        opacity: 0.86,
        stroke: theme.previewPanelStroke,
        "stroke-width": 1,
        "stroke-opacity": 0.22,
        filter: "url(#softPanelGlow)",
      }),
      text(panelX + 24, y + 30, `${remaining}%`, {
        size: 28,
        weight: 650,
        fill: theme.previewText,
      }),
      text(panelX + layout.kPanelWidth - 20, y + 23, data.label, {
        anchor: "end",
        size: 14,
        weight: 600,
        fill: theme.kThemeAccent,
      }),
      single("rect", {
        x: barX,
        y: barY,
        width: barWidth,
        height: 10,
        rx: 4,
        fill: theme.previewBarTrack,
        opacity: 1,
      }),
      single("rect", {
        x: barX,
        y: barY,
        width: fillWidth,
        height: 10,
        rx: 4,
        fill: color,
        filter: "url(#barGlow)",
      }),
      text(panelX + 24, y + 60, data.reset, {
        size: 12,
        weight: 500,
        fill: theme.previewDim,
      }),
    ].join("\n")
  );
}

/** Draws the centered loader-plus-status footer used by the firmware flex row. */
function usageFooter(layout, theme, status) {
  const estimatedStatusWidth = status.length * layout.kStatusTextFontPx * 0.56;
  const groupWidth = layout.kFooterLoaderWidth + layout.kFooterLoaderTextGap + estimatedStatusWidth;
  const startX = (layout.kScreenWidth - groupWidth) / 2;
  const dotY = 222;
  const textX = startX + layout.kFooterLoaderWidth + layout.kFooterLoaderTextGap;

  return el(
    "g",
    {},
    [
      single("circle", { cx: startX + 3, cy: dotY, r: 3, fill: theme.kThemeOrange }),
      single("circle", { cx: startX + 12, cy: dotY, r: 3, fill: theme.kThemeOrange, opacity: 0.45 }),
      single("circle", { cx: startX + 21, cy: dotY, r: 3, fill: theme.kThemeOrange, opacity: 0.25 }),
      text(textX, 226, status, {
        size: layout.kStatusTextFontPx,
        weight: 600,
        fill: theme.kThemeOrange,
      }),
    ].join("\n")
  );
}

/** Draws the usage screen shown in the README screenshots. */
function renderUsageScreen(layout, theme) {
  return svgDocument(layout, theme, [
    sharedHeader(layout, theme),
    text(layout.kScreenWidth / 2 + 8, 35, "ChatGPT", {
      anchor: "middle",
      size: 32,
      weight: 600,
      fill: theme.kThemeAccent,
    }),
    usagePanel(layout, theme, layout.kContentY, {
      percent: 8,
      label: "Session",
      reset: "Resets in 3h 35m",
    }),
    usagePanel(layout, theme, layout.kContentY + layout.kPanelHeight + layout.kPanelGap, {
      percent: 98,
      label: "Weekly",
      reset: "Resets in 3d 2h",
    }),
    usageFooter(layout, theme, "Quiet - 5h 8% / 7d 98%"),
  ]);
}

/** Draws the BLE info screen shown in the README screenshots. */
function renderBluetoothScreen(layout, theme, version) {
  const infoY = layout.kContentY;
  const resetY = layout.kContentY + 96;
  return svgDocument(layout, theme, [
    sharedHeader(layout, theme, "USB"),
    text(layout.kScreenWidth / 2 + 12, 34, "Info", {
      anchor: "middle",
      size: 28,
      weight: 600,
      fill: theme.kThemeAccent,
    }),
    single("rect", {
      x: layout.kMargin,
      y: infoY,
      width: layout.kPanelWidth,
      height: 86,
      rx: 6,
      fill: theme.previewUsagePanel,
      opacity: 0.86,
      stroke: theme.previewPanelStroke,
      "stroke-width": 1,
      "stroke-opacity": 0.22,
      filter: "url(#softPanelGlow)",
    }),
    text(layout.kMargin + 20 + layout.kInfoTextOffset, infoY + 30 + layout.kInfoTextOffset, "BLE Advertising", {
      size: 24,
      weight: 600,
      fill: theme.kThemeAmber,
    }),
    text(layout.kMargin + 20 + layout.kInfoTextOffset, infoY + 55 + layout.kInfoTextOffset, "Device: Neon Meter", {
      size: 14,
      weight: 500,
      fill: theme.previewDim,
    }),
    text(layout.kMargin + 20 + layout.kInfoTextOffset, infoY + 75 + layout.kInfoTextOffset, "Address: --:--:--:--:--:--", {
      size: 14,
      weight: 500,
      fill: theme.previewDim,
    }),
    single("rect", {
      x: layout.kMargin,
      y: resetY,
      width: layout.kPanelWidth,
      height: 54,
      rx: 6,
      fill: theme.previewUsagePanel,
      opacity: 0.86,
      stroke: theme.kThemeOrange,
      "stroke-width": 1,
      "stroke-opacity": 0.22,
      filter: "url(#orangeGlow)",
    }),
    text(layout.kScreenWidth / 2, resetY + 33, "Reset Bluetooth", {
      anchor: "middle",
      size: 16,
      weight: 600,
      fill: theme.kThemeOrange,
    }),
    text(layout.kScreenWidth / 2, layout.kScreenHeight - 14, `Neon Meter CoreS3 v${version}`, {
      anchor: "middle",
      size: 14,
      weight: 500,
      fill: theme.previewDim,
    }),
  ]);
}

/** Wraps screen content with common SVG definitions. */
function svgDocument(layout, theme, children) {
  return `<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" width="${layout.kScreenWidth}" height="${layout.kScreenHeight}" viewBox="0 0 ${layout.kScreenWidth} ${layout.kScreenHeight}" role="img">
  <title>Neon Meter firmware screen render</title>
  <defs>
    <filter id="cyanGlow" x="-60%" y="-60%" width="220%" height="220%">
      <feGaussianBlur stdDeviation="2.4" result="blur"/>
      <feMerge><feMergeNode in="blur"/><feMergeNode in="SourceGraphic"/></feMerge>
    </filter>
    <filter id="barGlow" x="-20%" y="-120%" width="140%" height="340%">
      <feGaussianBlur stdDeviation="1.8" result="blur"/>
      <feMerge><feMergeNode in="blur"/><feMergeNode in="SourceGraphic"/></feMerge>
    </filter>
    <filter id="softPanelGlow" x="-8%" y="-24%" width="116%" height="148%">
      <feDropShadow dx="0" dy="0" stdDeviation="2.5" flood-color="${theme.kThemeAccent}" flood-opacity="0.2"/>
    </filter>
    <filter id="orangeGlow" x="-8%" y="-28%" width="116%" height="156%">
      <feDropShadow dx="0" dy="0" stdDeviation="2.5" flood-color="${theme.kThemeOrange}" flood-opacity="0.22"/>
    </filter>
    <style>
      text { font-family: "Montserrat", "Avenir Next", "Segoe UI", sans-serif; letter-spacing: 0; }
    </style>
  </defs>
  <rect width="100%" height="100%" fill="${theme.previewBg}"/>
  ${children.join("\n  ")}
</svg>
`;
}

/** Generates all README screenshots. */
async function main() {
  const [themeSource, layoutSource, splashSource, packageSource] = await Promise.all([
    readProjectFile("src/theme.h"),
    readProjectFile("src/ui_layout.h"),
    readProjectFile("src/splash_animation.h"),
    readProjectFile("package.json"),
  ]);
  const theme = previewTheme(parseTheme(themeSource));
  const layout = parseConstants(splashSource, parseConstants(layoutSource));
  const packageInfo = JSON.parse(packageSource);

  await mkdir(assetsDir, { recursive: true });
  await writeFile(join(assetsDir, "neon-meter-usage-screen.svg"), renderUsageScreen(layout, theme));
  await writeFile(join(assetsDir, "neon-meter-bluetooth-screen.svg"), renderBluetoothScreen(layout, theme, packageInfo.version));
}

await main();
