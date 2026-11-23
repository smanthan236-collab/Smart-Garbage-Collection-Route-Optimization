
const API_URL = "http" + "://localhost:8080/api";


async function refreshAssignment() {
    const driverID = sessionStorage.getItem("smartwasteDriverId");

    const content = document.getElementById("assignmentContent");
    const message = document.getElementById("assignmentMessage");
    const completeButton = document.getElementById("completeCollectionButton");
    const actionMessage = document.getElementById("collectionActionMessage");

    if (completeButton) completeButton.disabled = true;

    if (!driverID) {
        content.innerHTML = `
            <div class="empty-state">
                Please login from the Driver Dashboard first.
                <br><br>
                <a href="index.html">Go to Dashboard</a>
            </div>
        `;
        return;
    }

    document.getElementById("sidebarDriverName").textContent =
        "Driver " + driverID;

    try {
        const response = await fetch(
            API_URL + "/driver/" + driverID + "/assignment"
        );

        const data = await response.json();

        if (!response.ok || !data.success) {
            throw new Error(data.message || "Unable to load assignment.");
        }

        document.getElementById("sidebarDriverName").textContent =
            data.driver.name;

        document.getElementById("assignedTruckStat").textContent =
            data.truck ? data.truck.id : "--";

        if (!data.assigned) {
            if (completeButton) completeButton.disabled = true;
            document.getElementById("assignedBinStat").textContent = "--";
            document.getElementById("binFillStat").textContent = "--";
            document.getElementById("garbageTypeStat").textContent = "--";

            content.innerHTML = `
                <div class="empty-state">
                    No active bin assignment is available for your truck.
                    Please contact the administrator.
                </div>
            `;

            message.hidden = true;
            return;
        }

        const bin = data.assignment;

        if (completeButton) {
            completeButton.disabled = false;

            completeButton.onclick = async function () {
                const confirmed = window.confirm(
                    "Confirm garbage from " + bin.binId +
                    " has actually been collected? This updates the database."
                );

                if (!confirmed) return;

                completeButton.disabled = true;
                completeButton.textContent = "Saving Collection...";

                try {
                    const result = await fetch(
                        API_URL + "/driver/" +
                        encodeURIComponent(driverID) + "/complete",
                        { method: "POST" }
                    );

                    const resultData = await result.json();

                    if (!result.ok || !resultData.success)
                        throw new Error(resultData.message || "Collection failed.");

                    if (actionMessage)
                        actionMessage.textContent = resultData.message;

                    await refreshAssignment();
                } catch (error) {
                    if (actionMessage)
                        actionMessage.textContent = error.message;

                    completeButton.disabled = false;
                } finally {
                    completeButton.textContent = "Mark Collection Complete";
                }
            };
        }

        const route = Array.isArray(bin.route)
            ? bin.route.map(String).join(" → ")
            : "Unavailable";
        const distance = Number(bin.distanceKm);

        document.getElementById("assignedBinStat").textContent =
            bin.binId;

        document.getElementById("binFillStat").textContent =
            Number(bin.fill).toFixed(1) + "%";

        document.getElementById("garbageTypeStat").textContent =
            bin.type;

        content.innerHTML = `
            <div class="assignment-task-grid">

                <div class="assignment-task-item">
                    <span>Bin Number</span>
                    <strong>${escapeHTML(bin.binId)}</strong>
                    <small>Assigned collection bin</small>
                </div>

                <div class="assignment-task-item">
                    <span>Collection Location</span>
                    <strong>${escapeHTML(bin.location)}</strong>
                    <small>Road network node: ${bin.locationNode}</small>
                </div>

                <div class="assignment-task-item">
                    <span>Garbage Type</span>
                    <strong>${escapeHTML(bin.type)}</strong>
                    <small>Recorded waste classification</small>
                </div>

                <div class="assignment-task-item">
                    <span>Estimated Garbage</span>
                    <strong>${Number(bin.estimatedGarbage).toFixed(1)} kg</strong>
                    <small>Based on recorded bin fill</small>
                </div>

            </div>

            <div class="assignment-task-grid" style="margin-top:16px">
                <div class="assignment-task-item">
                    <span>Shortest Route</span>
                    <strong>${escapeHTML(route)}</strong>
                    <small>Calculated using Dijkstra's algorithm</small>
                </div>
                <div class="assignment-task-item">
                    <span>Travel Distance</span>
                    <strong>${Number.isFinite(distance) && distance >= 0
                        ? distance.toFixed(1) + " km"
                        : "Unavailable"}</strong>
                    <small>Shortest road-network distance</small>
                </div>
            </div>
            <div class="progress-heading" style="margin-top:22px">
                <span>Bin Fill Level</span>
                <strong>${Number(bin.fill).toFixed(1)}%</strong>
            </div>

            <div class="progress-track">
                <div class="progress-fill"
                     style="width:${Math.min(Number(bin.fill), 100)}%">
                </div>
            </div>

            <p class="panel-note">
                Truck: ${escapeHTML(data.truck.id)}
                &nbsp; | &nbsp;
                Bin capacity: ${Number(bin.capacity).toFixed(1)} kg
            </p>
        `;

        message.textContent = "Assignment information loaded successfully.";
        message.className = "alert success";
        message.hidden = false;

    } catch (error) {
        console.error(error);

        content.innerHTML = `
            <div class="empty-state">
                ${escapeHTML(error.message)}
            </div>
        `;

        message.hidden = true;
    }
}

function escapeHTML(value) {
    return String(value ?? "").replace(/[&<>"']/g, function (char) {
        return {
            "&": "&amp;",
            "<": "&lt;",
            ">": "&gt;",
            '"': "&quot;",
            "'": "&#39;"
        }[char];
    });
}

document.addEventListener("DOMContentLoaded", refreshAssignment);
